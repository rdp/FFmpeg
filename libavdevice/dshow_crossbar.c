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

static const char *
GetPhysicalPinName(long pin_type)
{
    switch (pin_type)
    {
    case PhysConn_Video_Tuner:            return "Video Tuner";
    case PhysConn_Video_Composite:        return "Video Composite";
    case PhysConn_Video_SVideo:           return "S-Video";
    case PhysConn_Video_RGB:              return "Video RGB";
    case PhysConn_Video_YRYBY:            return "Video YRYBY";
    case PhysConn_Video_SerialDigital:    return "Video Serial Digital";
    case PhysConn_Video_ParallelDigital:  return "Video Parallel Digital";
    case PhysConn_Video_SCSI:             return "Video SCSI";
    case PhysConn_Video_AUX:              return "Video AUX";
    case PhysConn_Video_1394:             return "Video 1394";
    case PhysConn_Video_USB:              return "Video USB";
    case PhysConn_Video_VideoDecoder:     return "Video Decoder";
    case PhysConn_Video_VideoEncoder:     return "Video Encoder";

    case PhysConn_Audio_Tuner:            return "Audio Tuner";
    case PhysConn_Audio_Line:             return "Audio Line";
    case PhysConn_Audio_Mic:              return "Audio Microphone";
    case PhysConn_Audio_AESDigital:       return "Audio AES/EBU Digital";
    case PhysConn_Audio_SPDIFDigital:     return "Audio S/PDIF";
    case PhysConn_Audio_SCSI:             return "Audio SCSI";
    case PhysConn_Audio_AUX:              return "Audio AUX";
    case PhysConn_Audio_1394:             return "Audio 1394";
    case PhysConn_Audio_USB:              return "Audio USB";
    case PhysConn_Audio_AudioDecoder:     return "Audio Decoder";

    default:                              return "Unknown Type";
    }
}

static HRESULT 
setup_crossbar_options(IAMCrossbar *pXBar, int video_input_pin, int audio_input_pin, const char *device_name)
{
    HRESULT hr;
    long count_output, count_input;
    int i;
    
    av_log(NULL, AV_LOG_INFO, "Crossbar Switching Information for %s:\n", device_name); /* TODO only log if show_options set */
    hr = IAMCrossbar_get_PinCounts(pXBar, &count_output, &count_input);
    for (i = 0; i < count_output; i++)
    {
        long related_pin = -1, pin_type = -1, route_to_pin = -1;

        hr = IAMCrossbar_get_CrossbarPinInfo(pXBar, FALSE, i, &related_pin, &pin_type);
        if (pin_type == PhysConn_Video_VideoDecoder) {
            /* assume there is only one "Video (and one Audio) Decoder" output pin, and it's all we care about routing to...for now */
            if (video_input_pin != -1) {
                av_log(NULL, AV_LOG_DEBUG, "routing video input from pin %d\n", video_input_pin);
                if(IAMCrossbar_Route(pXBar, i, video_input_pin) != S_OK) {
                  av_log(NULL, AV_LOG_ERROR, "unable to route video input from pin %d\n", video_input_pin);
                  return AVERROR(EIO);
                }
            }
        } else if (pin_type == PhysConn_Audio_AudioDecoder) {
            if (audio_input_pin != -1) {
                av_log(NULL, AV_LOG_DEBUG, "routing audio input from pin %d\n", audio_input_pin);
                if(IAMCrossbar_Route(pXBar, i, audio_input_pin) != S_OK) {
                    av_log(NULL, AV_LOG_ERROR, "unable to route audio input from pin %d\n", audio_input_pin);
                    return AVERROR(EIO);
                }
            }
        } else {
            av_log(NULL, AV_LOG_WARNING, "unexpected output pin type, please report the type if you want to use this (%s)", GetPhysicalPinName(pin_type));
        }

        hr = IAMCrossbar_get_IsRoutedTo(pXBar, i, &route_to_pin);

        av_log(NULL, AV_LOG_INFO, "Crossbar Output pin %d: \"%s\" related output pin: %ld ", i, GetPhysicalPinName(pin_type), related_pin); /* like "Video Decoder" */
        
        av_log(NULL, AV_LOG_INFO, "current input pin: %ld ", route_to_pin);
        av_log(NULL, AV_LOG_INFO, "compatible input pins: ");

        for (int j = 0; j < count_input; j++)
        {
            hr = IAMCrossbar_CanRoute(pXBar, i, j);
            if (hr == S_OK) 
              av_log(NULL, AV_LOG_INFO ,"%d ", j);
        }
        av_log(NULL, AV_LOG_INFO, "\n");
    }

    for (i = 0; i < count_input; i++)
    {
        long related_pin, pin_type;
        hr = IAMCrossbar_get_CrossbarPinInfo(pXBar, TRUE, i, &related_pin, &pin_type);
        av_log(NULL, AV_LOG_INFO, "Crossbar input pin %d - \"%s\" ", i, GetPhysicalPinName(pin_type));
        av_log(NULL, AV_LOG_INFO, "related input pin: %ld\n", related_pin);
    }
    return S_OK;
}

void 
dshow_show_filter_properties(IBaseFilter *pFilter) {
    ISpecifyPropertyPages *pProp;
    HRESULT hr = IBaseFilter_QueryInterface(pFilter, &IID_ISpecifyPropertyPages, (void **)&pProp);
    if (SUCCEEDED(hr)) 
    {
        FILTER_INFO FilterInfo;
        IUnknown *pFilterUnk;
        CAUUID caGUID;

        hr = IBaseFilter_QueryFilterInfo(pFilter, &FilterInfo); 
        IBaseFilter_QueryInterface(pFilter, &IID_IUnknown, (void **)&pFilterUnk);
        ISpecifyPropertyPages_GetPages(pProp, &caGUID);
        ISpecifyPropertyPages_Release(pProp);
        OleCreatePropertyFrame(NULL, 0, 0, FilterInfo.achName, 1, &pFilterUnk, caGUID.cElems, 
            caGUID.pElems, 0, 0, NULL);

        IUnknown_Release(pFilterUnk);
        if (FilterInfo.pGraph)
          IFilterGraph_Release(FilterInfo.pGraph); 
        CoTaskMemFree(caGUID.pElems);
    } else {
        av_log(NULL, AV_LOG_WARNING, "unable to show properties for a requested filter");
    }
}

HRESULT 
dshow_try_setup_crossbar_options(ICaptureGraphBuilder2 *graph_builder2, IBaseFilter *device_filter, 
    int crossbar_video_input_pin_number, int crossbar_audio_input_pin_number, const char *device_name) {
    IAMCrossbar *pCrossBar = NULL;
    HRESULT r;
    r = ICaptureGraphBuilder2_FindInterface(graph_builder2, &LOOK_UPSTREAM_ONLY, (const GUID *) NULL,
            (IBaseFilter *) device_filter, &IID_IAMCrossbar, (void**) &pCrossBar);
    if (r == S_OK) {
        /* It found a cross bar device was inserted, optionally configure it */
        r = setup_crossbar_options(pCrossBar, crossbar_video_input_pin_number, 
            crossbar_audio_input_pin_number, device_name);
        IAMCrossbar_Release(pCrossBar);
    } else {
        /* no crossbar to setup is OK */
        r = S_OK;
    }
    return r;
}
