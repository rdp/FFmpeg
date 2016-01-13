/*
 * DirectShow capture interface
 * Copyright (c) 2015 Roger Pack
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "dshow_capture.h"
#include <tuner.h>
//#include <uuids.h>

// local defines:
static const CLSID CLSID_NetworkProvider =
    {0xB2F3A67C,0x29DA,0x4C78,{0x88,0x31,0x09,0x1E,0xD5,0x09,0xA4,0x75}};
static const GUID KSCATEGORY_BDA_NETWORK_TUNER =
    {0x71985f48,0x1ca1,0x11d3,{0x9c,0xc8,0x00,0xc0,0x4f,0x79,0x71,0xe0}};
static const GUID KSCATEGORY_BDA_RECEIVER_COMPONENT    =
    {0xFD0A5AF4,0xB41D,0x11d2,{0x9c,0x95,0x00,0xc0,0x4f,0x79,0x71,0xe0}};
static const CLSID CLSID_BDA_MPEG2_Transport_Information_Filter =
    {0xFC772AB0,0x0C7F,0x11D3,{0x8F,0xF2,0x00,0xA0,0xC9,0x22,0x4C,0xF4}};

static int
dshow_lookup_pin(AVFormatContext *avctx, IBaseFilter *filter, PIN_DIRECTION pin_direction, IPin **discovered_pin, const char *lookup_pin_name, const char *filter_descriptive_text);
static int
dshow_cycle_dtv_devices(AVFormatContext *avctx, enum dshowDtvFilterType devtype, ICreateDevEnum *devenum, IBaseFilter **pfilter);
static int
dshow_connect_bda_pins(AVFormatContext *avctx, IBaseFilter *source, const char *src_pin_name, IBaseFilter *destination, const char *dest_pin_name, IPin **lookup_pin, const char *lookup_pin_name );

HRESULT setup_dshow_dtv(AVFormatContext *avctx) {     
        struct dshow_ctx *ctx = avctx->priv_data;
        ICreateDevEnum *devenum = NULL;
        IBaseFilter *bda_net_provider = NULL;
        IBaseFilter *bda_source_device = NULL;
        IBaseFilter *bda_receiver_device = NULL;
        IBaseFilter *bda_filter_supplying_stream = NULL;
        IBaseFilter *bda_infinite_tee = NULL;
        IBaseFilter *bda_mpeg2_demux = NULL;
        IBaseFilter *bda_mpeg2_info = NULL;
        libAVPin *capture_pin = NULL;
        libAVFilter *capture_filter = NULL;
        IScanningTuner *scanning_tuner = NULL;
        IEnumTuningSpaces *tuning_space_enum = NULL;
        ITuneRequest *tune_request = NULL;
        ITuningSpace *tuning_space = NULL;
        IDVBTuneRequest *dvb_tune_request = NULL;
        IDVBTuningSpace2 *dvb_tuning_space2 = NULL;
        IDVBSTuningSpace *dvbs_tuning_space = NULL;
        IATSCChannelTuneRequest *atsc_tune_request = NULL;
        ILocator *def_locator = NULL;
        IDVBCLocator *dvbc_locator = NULL;
        IDVBTLocator *dvbt_locator = NULL;
        IDVBSLocator *dvbs_locator = NULL;
        IATSCLocator *atsc_locator = NULL;
        GUID CLSIDNetworkType = GUID_NULL;
        GUID tuning_space_network_type = GUID_NULL;
        IPin *bda_mpeg_video_pin = NULL;
        ICaptureGraphBuilder2 *graph_builder2 = NULL;
        int use_infinite_tee_ts_stream = 1; // set to 1 to allow capture of "raw" MPEG TS incoming stream
        int r;
        IGraphBuilder *graph = ctx->graph;
        ILocator *locator_used;
        const wchar_t *filter_name[2] = { L"Audio capture filter unused", L"DTV Video capture filter" };
		
        r = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                             &IID_ICreateDevEnum, (void **) &devenum);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not enumerate system devices.\n");
            goto error;
        }

        // add network provider

        if( ctx->dtv == 1 ) {
            av_log(avctx, AV_LOG_INFO, "NetworkType: DVB-C\n" );
            CLSIDNetworkType = CLSID_DVBCNetworkProvider;
        } else if( ctx->dtv == 2 ) {
            av_log(avctx, AV_LOG_INFO, "NetworkType: DVB-T\n" );
            CLSIDNetworkType = CLSID_DVBTNetworkProvider;
        } else if( ctx->dtv == 3 ) {
            av_log(avctx, AV_LOG_INFO, "NetworkType: DVB-S\n" );
            CLSIDNetworkType = CLSID_DVBSNetworkProvider;
        } else if( ctx->dtv == 4 ) {
            av_log(avctx, AV_LOG_INFO, "NetworkType: ATSC\n" );
            CLSIDNetworkType = CLSID_ATSCNetworkProvider;
        }else{
            av_log(avctx, AV_LOG_INFO, "Unknown NetworkType: %d\n", ctx->dtv );
            goto error;
        }
		
		av_log(avctx, AV_LOG_INFO, "Attempting to setup DTV tuner tune_freq=%ld dvbt_tune_bandwidth_mhz=%d "
            "atsc_physical_channel=%d dtv_tune_modulation=%d receiver_component=%s\n",		
		    ctx->tune_freq, ctx->dvbt_tune_bandwidth_mhz, ctx->atsc_physical_channel, ctx->dtv_tune_modulation, ctx->receiver_component);


        r = CoCreateInstance(&CLSIDNetworkType, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IBaseFilter, (void **) &bda_net_provider);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create BDA network provider\n");
            goto error;
        }

        r = IGraphBuilder_AddFilter(graph, bda_net_provider, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA network provider to graph.\n");
            goto error;
        }

        ///add dtv filters

        if (ctx->list_devices) {
            av_log(avctx, AV_LOG_INFO, "BDA tuners:\n");
            dshow_cycle_dtv_devices(avctx, NetworkTuner, devenum, NULL);
            av_log(avctx, AV_LOG_INFO, "BDA receivers:\n");
            dshow_cycle_dtv_devices(avctx, ReceiverComponent, devenum, NULL);
            goto error;
        }

        r = IBaseFilter_QueryInterface(bda_net_provider, &IID_IScanningTuner, (void**) &scanning_tuner);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not query scanning tuner.\n");
            goto error;
        }
        ctx->scanning_tuner = scanning_tuner;

        r = IScanningTuner_EnumTuningSpaces(scanning_tuner, &tuning_space_enum);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not get tuning space enum.\n");
            goto error;
        }

        tuning_space_network_type = GUID_NULL;

        while (S_OK == IEnumTuningSpaces_Next(tuning_space_enum, 1, &tuning_space, NULL))
        {
            BSTR bstr_name = NULL;
            char *psz_bstr_name = NULL;

            r = ITuningSpace_get_FriendlyName(tuning_space, &bstr_name);
            if(r != S_OK) {
                /* should never fail on a good tuning space */
                av_log(avctx, AV_LOG_ERROR, "Cannot get UniqueName for Tuning Space: r=0x%8x\n", r );
                goto error;
            }

            psz_bstr_name = dup_wchar_to_utf8(bstr_name);
            SysFreeString(bstr_name);
            av_log(avctx, AV_LOG_INFO, "Using Tuning Space: %s\n", psz_bstr_name );


            r = ITuningSpace_get__NetworkType(tuning_space, &tuning_space_network_type);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Error acquiring tuning space type.\n");
                goto error;
            }

            if ( IsEqualGUID(&tuning_space_network_type, &CLSIDNetworkType) )
                    break;

            tuning_space_network_type = GUID_NULL;
        }


        if (IsEqualGUID(&tuning_space_network_type, &GUID_NULL)){
            av_log(avctx, AV_LOG_ERROR, "Could not found right type tuning space.\n");
            goto error;
        }

        r = IScanningTuner_put_TuningSpace(scanning_tuner, tuning_space);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could put in tuning space to scanning tuner\n");
            goto error;
        }

        r = ITuningSpace_get_DefaultLocator(tuning_space, &def_locator);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not get default locator\n");
            goto error;
        }

        r = ITuningSpace_CreateTuneRequest(tuning_space, &tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create test tune request\n");
            goto error;
        }

        r = ITuneRequest_put_Locator(tune_request, def_locator);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could put default locator\n");
            goto error;
        }

        // this just means "can this tune request theoretically work with this card" 
        // not whether the frequency is good.
        r = IScanningTuner_Validate(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Error validating tune request: r=0x%8x\n", r);
            goto error;
        }

        r = IScanningTuner_put_TuneRequest(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not put tune request\n");
            goto error;
        }

        //////////////////////////////////////// add network tuner
        
        if ((r = dshow_cycle_dtv_devices(avctx, NetworkTuner, devenum, &bda_source_device)) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Could not find BDA tuner.\n");
            goto error;
        }

        bda_filter_supplying_stream = bda_source_device;

        r = IGraphBuilder_AddFilter(graph, bda_source_device, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA tuner to graph.\n");
            goto error;
        }

        av_log(avctx, AV_LOG_INFO, "BDA tuner added\n");

        ///connect network provider and tuner

        r = dshow_connect_bda_pins(avctx, bda_net_provider, NULL, bda_source_device, NULL, NULL, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not connect network provider to tuner. Trying generic Network Provider instead.\n");
            r = IGraphBuilder_RemoveFilter(graph, bda_net_provider);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not remove old BDA network provider from graph.\n");
                goto error;
            }

            r = CoCreateInstance(&CLSID_NetworkProvider, NULL, CLSCTX_INPROC_SERVER,
                                 &IID_IBaseFilter, (void **) &bda_net_provider);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not create generic BDA network provider\n");
                goto error;
            }

            r = IGraphBuilder_AddFilter(graph, bda_net_provider, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not add generic BDA network provider to graph.\n");
                goto error;
            }

            r = dshow_connect_bda_pins(avctx, bda_net_provider, NULL, bda_source_device, NULL, NULL, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not connect generic network provider to tuner.\n");
                goto error;
            }
            av_log(avctx, AV_LOG_INFO, "Generic Network Provider worked in its place\n");
        }

        ///---add receive component if requested
        if (ctx->receiver_component) {
            // find right named device
            if ((r = dshow_cycle_dtv_devices(avctx, ReceiverComponent, devenum, &bda_receiver_device)) < 0) {
                goto error;
            }

            r = IGraphBuilder_AddFilter(graph, bda_receiver_device, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not add BDA receiver to graph.\n");
                goto error;
            }

            av_log(avctx, AV_LOG_INFO, "BDA Receiver Component added\n");

            ///connect tuner to receiver component

            r = dshow_connect_bda_pins(avctx, bda_source_device, NULL, bda_receiver_device, NULL, NULL, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not connect tuner to receiver component.\n");
                goto error;
            }
            bda_filter_supplying_stream = bda_receiver_device; 
        }

        // create infinite tee so we can just grab the MPEG TS stream -> ffmpeg
        r = CoCreateInstance(&CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IBaseFilter, (void **) &bda_infinite_tee);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create BDA infinite tee\n");
            goto error;
        }
        r = IGraphBuilder_AddFilter(graph, bda_infinite_tee, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA infinite tee to graph.\n");
            goto error;
        }

        r = dshow_connect_bda_pins(avctx, bda_filter_supplying_stream, NULL, bda_infinite_tee, NULL, NULL, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not connect bda pin to infinite tee.\n");
            goto error;
        }
        
        // still need the MS MPEG2 demux and "IB Input" filter to that, so that tuning occurs

        r = CoCreateInstance(&CLSID_MPEG2Demultiplexer, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IBaseFilter, (void **) &bda_mpeg2_demux);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create BDA mpeg2 demux\n");
            goto error;
       }

        r = IGraphBuilder_AddFilter(graph, bda_mpeg2_demux, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA mpeg2 demux to graph.\n");
            goto error;
        }
        if (use_infinite_tee_ts_stream)
          r = dshow_connect_bda_pins(avctx, bda_infinite_tee, "Output1", bda_mpeg2_demux, NULL, NULL, NULL);
        else
          r = dshow_connect_bda_pins(avctx, bda_infinite_tee, "Output1", bda_mpeg2_demux, NULL, &bda_mpeg_video_pin, "3"); // TODO fix me! 003 also
        if (r != S_OK) {
            goto error;
        }
        // now the infinite tee will now have an "Output2" named pin since we just "used" Output1

        if (use_infinite_tee_ts_stream) {
          r = dshow_lookup_pin(avctx, bda_infinite_tee, PINDIR_OUTPUT, &bda_mpeg_video_pin, "Output2", "split mpeg tee pin");
          if (r != S_OK) {
            goto error;
          }
        }

        // add DBA MPEG2 Transport information filter to MS demux pin "1"
        
        r = CoCreateInstance(&CLSID_BDA_MPEG2_Transport_Information_Filter, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IBaseFilter, (void **) &bda_mpeg2_info);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create BDA mpeg2 info filter\n");
            goto error;
        }

        r = IGraphBuilder_AddFilter(graph, bda_mpeg2_info, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA mpeg2 transport info filter to graph.\n");
            goto error;
        }

        r = dshow_connect_bda_pins(avctx, bda_mpeg2_demux, "1", bda_mpeg2_info, "IB Input", NULL, NULL);
        if (r != S_OK) {
            r = dshow_connect_bda_pins(avctx, bda_mpeg2_demux, "001", bda_mpeg2_info, "IB Input", NULL, NULL);
            if (r != S_OK) {
              av_log(avctx, AV_LOG_ERROR, "Could not connect mpeg2 demux to mpeg2 transport information filter.\n");
              goto error;
            }
        }

        /////////////////////////////////
        ///////////////DTV tuning

        r = IScanningTuner_get_TuneRequest(scanning_tuner, &tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not get tune request -- Trying to create one\n");
            r = ITuningSpace_CreateTuneRequest(tuning_space, &tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not create tune request\n");
                goto error;
            }
        }

        //// if tuningspace == DVB-X (i.e. not ATSC)
        if (ctx->dtv > 0 && ctx->dtv < 4) {

            r = ITuneRequest_QueryInterface(tune_request, &IID_IDVBTuneRequest, (void **) &dvb_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not query DVBTuneRequest.\n");
                goto error;
            }

            IDVBTuneRequest_put_ONID(dvb_tune_request, -1 );
            IDVBTuneRequest_put_SID(dvb_tune_request, -1 );
            IDVBTuneRequest_put_TSID(dvb_tune_request, -1 );

            //// if tuningspace  == DVB-C
            if (ctx->dtv == 1){
                // untested
                r = CoCreateInstance(&CLSID_DVBCLocator, NULL, CLSCTX_INPROC_SERVER,
                                     &IID_IDVBCLocator, (void **) &dvbc_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not create DVB-C Locator\n");
                    goto error;
                }

                r = IScanningTuner_get_TuningSpace(scanning_tuner, &tuning_space);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not get tuning space from scanning tuner\n");
                    goto error;
                }

                r = ITuningSpace_QueryInterface(tuning_space, &IID_IDVBTuningSpace2, (void **) &dvb_tuning_space2);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not query DVBTuningSpace2\n");
                    goto error;
                }


                r = IDVBTuningSpace2_put_SystemType(dvb_tuning_space2, DVB_Cable);
                if(r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Cannot set dvb tuning space2 systyp\n");
                    goto error;
                }
                locator_used = (ILocator *) dvbc_locator;
            }

            //// if tuningspace  == DVB-T
            if (ctx->dtv == 2){
                r = CoCreateInstance(&CLSID_DVBTLocator, NULL, CLSCTX_INPROC_SERVER,
                                     &IID_IDVBTLocator, (void **) &dvbt_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not create DVB-T Locator\n");
                    goto error;
                }

                r = IScanningTuner_get_TuningSpace(scanning_tuner, &tuning_space);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not get tuning space from scanning tuner\n");
                    goto error;
                }

                r = ITuningSpace_QueryInterface(tuning_space, &IID_IDVBTuningSpace2, (void **) &dvb_tuning_space2);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not query DVBTuningSpace2\n");
                    goto error;
                }


                r = IDVBTuningSpace2_put_SystemType(dvb_tuning_space2, DVB_Terrestrial);
                if(r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Cannot set dvb tuning space2 systyp\n");
                    goto error;
                }
 
                if (ctx->dvbt_tune_bandwidth_mhz > 0) {
                    av_log(avctx, AV_LOG_INFO, "Setting dvbt_tune_bandwidth_mhz=%d\n", ctx->dvbt_tune_bandwidth_mhz);
                    r = IDVBTLocator_put_Bandwidth(dvbt_locator, ctx->dvbt_tune_bandwidth_mhz);
                    if (r != S_OK) {
                        av_log(avctx, AV_LOG_ERROR, "Could not set DVB-T bandwidth\n");
                        goto error;
                    }
                }
                locator_used = (ILocator *) dvbt_locator;
            }
            //// if tuningspace  == DVB-S
            if (ctx->dtv==3){
                r = CoCreateInstance(&CLSID_DVBSLocator, NULL, CLSCTX_INPROC_SERVER,
                                     &IID_IDVBSLocator, (void **) &dvbs_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not create DVB-S Locator\n");
                    goto error;
                }

                r = IScanningTuner_get_TuningSpace(scanning_tuner, &tuning_space);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not get tuning space from scanning tuner\n");
                    goto error;
                }

                r = ITuningSpace_QueryInterface(tuning_space, &IID_IDVBSTuningSpace, (void **) &dvbs_tuning_space);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not query DVBSTuningSpace\n");
                    goto error;
                }


                r = IDVBSTuningSpace_put_SystemType(dvbs_tuning_space, DVB_Satellite);
                if(r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Cannot set dvbs tuning space systyp\n");
                    goto error;
                }
                locator_used = (ILocator *) dvbs_locator;
            }
        } else if (ctx->dtv == 4) {
            ////  ATSC

            r = ITuneRequest_QueryInterface(tune_request, &IID_IATSCChannelTuneRequest, (void **) &atsc_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not query ATSCChannelTuneRequest.\n");
                goto error;
            }

            r = CoCreateInstance(&CLSID_ATSCLocator, NULL, CLSCTX_INPROC_SERVER,
                                 &IID_IATSCLocator, (void **) &atsc_locator);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not create ATSC Locator\n");
                goto error;
            }
            if (ctx->atsc_physical_channel) {
                r = IATSCLocator_put_PhysicalChannel(atsc_locator, ctx->atsc_physical_channel);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not specify ATSC physical channel %d\n", ctx->atsc_physical_channel);
                    goto error;
                }               
            }
            locator_used = (ILocator *) atsc_locator;
        } else {
            av_log(avctx, AV_LOG_ERROR, "unknown tuning type %d\n", ctx->dtv);
            goto error;
        }
		
		if (ctx->dtv_tune_modulation > 0) {
           r = ILocator_put_Modulation(locator_used, ctx->dtv_tune_modulation);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not set modulation %d\n", ctx->dtv_tune_modulation);
                goto error;
            }			
		}
        
        if (ctx->tune_freq>0){
            r = ILocator_put_CarrierFrequency(locator_used, ctx->tune_freq );
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not set tune freq %ld\n", ctx->tune_freq);
                goto error;
            }
        } else
			av_log(avctx, AV_LOG_ERROR, "no tune frequency requested, will probably be tuning to some default frequency\n");

        r = ITuneRequest_put_Locator(tune_request, locator_used);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not put locator on tune request\n");
            goto error;
        }

        r = IScanningTuner_Validate(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Error validating tune request\n");
            goto error;
        }

        r = IScanningTuner_put_TuneRequest(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not set put tune request to scanning tuner\n");
            goto error;
        }
        
        //////////////////////////////////////////// add to Graph

        capture_filter = libAVFilter_Create(avctx, dshow_frame_callback, VideoDevice);
        if (!capture_filter) {
            av_log(avctx, AV_LOG_ERROR, "Could not create grabber filter.\n");
            goto error;
        }
        ctx->capture_filter[VideoDevice] = capture_filter;

        r = IGraphBuilder_AddFilter(graph, (IBaseFilter *) capture_filter,
                                    filter_name[VideoDevice]);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add capture filter to graph\n");
            goto error;
        }

        capture_pin = capture_filter->pin;
        libAVPin_AddRef(capture_filter->pin);
        ctx->capture_pin[VideoDevice] = capture_pin;

        r = CoCreateInstance(&CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                             &IID_ICaptureGraphBuilder2, (void **) &graph_builder2);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create CaptureGraphBuilder2\n");
            goto error;
        }

        r = ICaptureGraphBuilder2_SetFiltergraph(graph_builder2, graph);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not set graph for CaptureGraphBuilder2\n");
            goto error;
        }

        r = ICaptureGraphBuilder2_RenderStream(graph_builder2, NULL, NULL, (IUnknown *) bda_mpeg_video_pin, NULL /* no intermediate filter */,
            (IBaseFilter *) capture_filter); /* connect pins, optionally insert intermediate filters like crossbar if necessary */
       // this in essence just connected bda_mpeg_video_pin with capture_filter->capture_pin no real need for RenderStream

        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not RenderStream to connect pins\n");
            goto error;
        }


        if (graph_builder2 != NULL)
            ICaptureGraphBuilder2_Release(graph_builder2);

        if ((r = dshow_add_device(avctx, VideoDevice)) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Could not add video device.\n");
            goto error;
        }
		
		av_log(avctx, AV_LOG_INFO, "DTV Video capture filter successfully added ot graph\n");

error:
        if (devenum)
            ICreateDevEnum_Release(devenum);
        return r;
}   
        
        
/* dshow_find_pin given a filter, direction and optional pin name, return a ref to that pin
 * note this does add a reference to the pin returned...
*/
static int
dshow_lookup_pin(AVFormatContext *avctx, IBaseFilter *filter, PIN_DIRECTION pin_direction, IPin **discovered_pin, const char *lookup_pin_name, const char *filter_descriptive_text) {
    IEnumPins *pins = 0;
    IPin *pin = NULL;
    IPin *local_discovered_pin = NULL; // for easier release checking
    int r;

    r = IBaseFilter_EnumPins(filter, &pins);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not enumerate filter %s pins.\n", filter_descriptive_text);
        return AVERROR(EIO);
    }
    while (!local_discovered_pin && IEnumPins_Next(pins, 1, &pin, NULL) == S_OK) {
        char *name_buf;
        PIN_INFO info = {0};

        IPin_QueryPinInfo(pin, &info);
        IBaseFilter_Release(info.pFilter);
        name_buf = dup_wchar_to_utf8(info.achName);
        av_log(avctx, AV_LOG_DEBUG, "Filter %s pin [%s] has direction %d wanted direction %d\n", filter_descriptive_text, name_buf, info.dir, pin_direction);

        if (info.dir == pin_direction) 
            if ( (lookup_pin_name == NULL) ||                                             //if input name empty
                    ((lookup_pin_name) && !(strcmp(name_buf,lookup_pin_name))) )     //or input name not empty and equal to the current pin
                  local_discovered_pin = pin;
        if (!local_discovered_pin)
          IPin_Release(pin);
    }
    IEnumPins_Release(pins);

    if (!local_discovered_pin) {
        if (lookup_pin_name)
            av_log(avctx, AV_LOG_ERROR, "Filter %s doesn't have pin with direction %d named \"%s\"\n", filter_descriptive_text, pin_direction, lookup_pin_name);
        else
            av_log(avctx, AV_LOG_ERROR, "Filter %s doesn't have pin with direction %d\n", filter_descriptive_text, pin_direction);
        return AVERROR(EIO);
    }
    *discovered_pin = local_discovered_pin;
    return 0; // success
}

/* dshow_connect_bda_pins connects [source] filter's output pin named [src_pin_name] to [destination] filter's input pin named [dest_pin_name]
 * and provides the [destination] filter's output pin named [lookup_pin_name] to a pin ptr [lookup_pin]
 * pin names and lookup_pin can be NULL if not needed/doesn't care
*/
static int
dshow_connect_bda_pins(AVFormatContext *avctx, IBaseFilter *source, const char *src_pin_name, IBaseFilter *destination, const char *dest_pin_name, IPin **lookup_pin, const char *lookup_pin_name )
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IGraphBuilder *graph = NULL;
    IPin *pin_out = NULL;
    IPin *pin_in = NULL;
    int r;

    graph = ctx->graph;

    if (!graph || !source || !destination)
    {
        av_log(avctx, AV_LOG_ERROR, "Missing graph component.\n");
        return AVERROR(EIO);
    }
    r = dshow_lookup_pin(avctx, source, PINDIR_OUTPUT, &pin_out, src_pin_name, "source filter");
    if (r != S_OK) {
        return r;
    }
    r = dshow_lookup_pin(avctx, destination, PINDIR_INPUT, &pin_in, dest_pin_name, "dest filter");
    if (r != S_OK) {
        return r;
    }

    ///connect pins

    r = IGraphBuilder_Connect(graph, pin_out, pin_in);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not connect pins.\n");
        return AVERROR(EIO);
    }

    if (lookup_pin != NULL) {
        r = dshow_lookup_pin(avctx, destination, PINDIR_OUTPUT, lookup_pin, lookup_pin_name, "outgoing pin on destination");
        if (r != S_OK) {
            return r;
        }
    }

    return 0;
}


/**
 * Cycle through available devices using the device enumerator devenum,
 * retrieve the device with type specified by devtype and return the
 * pointer to the object found in *pfilter.
 * If pfilter is NULL, list all device names.
 */
static int
dshow_cycle_dtv_devices(AVFormatContext *avctx, enum dshowDtvFilterType devtype, ICreateDevEnum *devenum, IBaseFilter **pfilter)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IBaseFilter *device_filter = NULL;
    IEnumMoniker *classenum = NULL;
    IMoniker *m = NULL;
    const char *device_name = (devtype == NetworkTuner) ? ctx->device_name[VideoDevice] : ctx->receiver_component;
    int skip = ctx->video_device_number;
    int r;

    const GUID *device_guid = (devtype == NetworkTuner) ? &KSCATEGORY_BDA_NETWORK_TUNER : &KSCATEGORY_BDA_RECEIVER_COMPONENT;
    const char *devtypename = "dtv";
    const char *sourcetypename = (devtype == NetworkTuner) ? "BDA Netwok Tuner" : "BDA Receiver Component";

    r = ICreateDevEnum_CreateClassEnumerator(devenum, device_guid,
                                             (IEnumMoniker **) &classenum, 0);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not enumerate %s devices (or none found).\n",
               devtypename);
        return AVERROR(EIO);
    }

    while (!device_filter && IEnumMoniker_Next(classenum, 1, &m, NULL) == S_OK) {
        IPropertyBag *bag = NULL;
        char *friendly_name = NULL;
        char *unique_name = NULL;
        VARIANT var;
        IBindCtx *bind_ctx = NULL;
        LPOLESTR olestr = NULL;
        LPMALLOC co_malloc = NULL;
        int i;

        r = CoGetMalloc(1, &co_malloc);
        if (r = S_OK)
            goto fail1;
        r = CreateBindCtx(0, &bind_ctx);
        if (r != S_OK)
            goto fail1;
        /* GetDisplayname works for both video and audio, DevicePath doesn't */
        r = IMoniker_GetDisplayName(m, bind_ctx, NULL, &olestr);
        if (r != S_OK)
            goto fail1;
        unique_name = dup_wchar_to_utf8(olestr);
        /* replace ':' with '_' since we use : to delineate between sources */
        for (i = 0; i < strlen(unique_name); i++) {
            if (unique_name[i] == ':')
                unique_name[i] = '_';
        }

        r = IMoniker_BindToStorage(m, 0, 0, &IID_IPropertyBag, (void *) &bag);
        if (r != S_OK)
            goto fail1;

        var.vt = VT_BSTR;
        r = IPropertyBag_Read(bag, L"FriendlyName", &var, NULL);
        if (r != S_OK)
            goto fail1;
        friendly_name = dup_wchar_to_utf8(var.bstrVal);

        if (pfilter) {
            av_log(avctx, AV_LOG_DEBUG, "comparing requested %s to device name %s\n", device_name, friendly_name);
            if (strcmp(device_name, friendly_name) && strcmp(device_name, unique_name))
                goto fail1;

            if (!skip--) {
                r = IMoniker_BindToObject(m, 0, 0, &IID_IBaseFilter, (void *) &device_filter);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Unable to BindToObject for %s\n", device_name);
                    goto fail1;
                }
            }
        } else {
            av_log(avctx, AV_LOG_INFO, " \"%s\"\n", friendly_name);
            av_log(avctx, AV_LOG_INFO, "    Alternative name \"%s\"\n", unique_name);
        }

fail1:
        if (olestr && co_malloc)
            IMalloc_Free(co_malloc, olestr);
        if (bind_ctx)
            IBindCtx_Release(bind_ctx);
        av_free(friendly_name);
        av_free(unique_name);
        if (bag)
            IPropertyBag_Release(bag);
        IMoniker_Release(m);
    }

    IEnumMoniker_Release(classenum);

    if (pfilter) {
        if (!device_filter) {
            av_log(avctx, AV_LOG_ERROR, "Could not find %s device with name [%s] among source devices of type %s.\n",
                   devtypename, device_name, sourcetypename);
            return AVERROR(EIO);
        }
        *pfilter = device_filter;
    }

    return 0;
}

long dshow_get_signal_strength(AVFormatContext *h) {
    struct dshow_ctx *ctx = h->priv_data;
	long signal_strength;
    int hr;
    hr = ITuner_get_SignalStrength(ctx->scanning_tuner, &signal_strength);
    if (hr == S_OK)
       return signal_strength;
    else {
      av_log(h, AV_LOG_ERROR, "unable to determine signal strength (%d)\n", hr);
	  return 0;
	}
}
