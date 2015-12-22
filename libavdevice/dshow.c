/*
 * Directshow capture interface
 * Copyright (c) 2010 Ramiro Polla
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
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "libavformat/internal.h"
#include "libavformat/riff.h"
#include "avdevice.h"
#include "libavcodec/raw.h"
#include "objidl.h"
#include "shlwapi.h"
#include "tuner.h"
#include "bdadefs.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h" // avstrstart

static const CLSID CLSID_NetworkProvider =
    {0xB2F3A67C,0x29DA,0x4C78,{0x88,0x31,0x09,0x1E,0xD5,0x09,0xA4,0x75}};
static const GUID KSCATEGORY_BDA_NETWORK_TUNER =
    {0x71985f48,0x1ca1,0x11d3,{0x9c,0xc8,0x00,0xc0,0x4f,0x79,0x71,0xe0}};
static const GUID KSCATEGORY_BDA_RECEIVER_COMPONENT    =
    {0xFD0A5AF4,0xB41D,0x11d2,{0x9c,0x95,0x00,0xc0,0x4f,0x79,0x71,0xe0}};
static const GUID KSCATEGORY_BDA_TRANSPORT_INFORMATION =
    {0xa2e3074f,0x6c3d,0x11d3,{0xb6,0x53,0x00,0xc0,0x4f,0x79,0x49,0x8e}};
static const GUID KSDATAFORMAT_SUBTYPE_BDA_MPEG2_TRANSPORT_LOCAL =
    {0xf4aeb342,0x0329,0x4fdd,{0xa8,0xfd,0x4a,0xff,0x49,0x26,0xc9,0x78}};

//const CLSID CLSID_MPEG2Demultiplexer =
//    {0xAFB6C280,0x2C41,0x11D3,{0x8A,0x60,0x00,0x00,0xF8,0x1E,0x0E,0x4A}};
static const CLSID CLSID_BDA_MPEG2_Transport_Informatiuon_Filter =
    {0xFC772AB0,0x0C7F,0x11D3,{0x8F,0xF2,0x00,0xA0,0xC9,0x22,0x4C,0xF4}};
static const CLSID CLSID_MS_DTV_DVD_Decoder =
    {0x212690FB,0x83E5,0x4526,{0x8F,0xD7,0x74,0x47,0x8B,0x79,0x39,0xCD}}; //ms dtv decoder



static enum AVPixelFormat dshow_pixfmt(DWORD biCompression, WORD biBitCount)
{
    enum AVPixelFormat out;
    switch(biCompression) {
    case BI_BITFIELDS:
    case BI_RGB:
        switch(biBitCount) { /* 1-8 are untested */
            case 1:
                return AV_PIX_FMT_MONOWHITE;
            case 4:
                return AV_PIX_FMT_RGB4;
            case 8:
                return AV_PIX_FMT_RGB8;
            case 16:
                return AV_PIX_FMT_RGB555;
            case 24:
                return AV_PIX_FMT_BGR24;
            case 32:
                return AV_PIX_FMT_0RGB32;
        }
    }
    out = avpriv_find_pix_fmt(avpriv_get_raw_pix_fmt_tags(), biCompression); // all others
    if (out == AV_PIX_FMT_NONE)
      av_log(NULL, AV_LOG_ERROR, "unable to determine pixel format? %ld %d\n", biCompression, biBitCount);
    return out;
}

static int
dshow_read_close(AVFormatContext *s)
{
    struct dshow_ctx *ctx = s->priv_data;
    AVPacketList *pktl;

    if (ctx->control) {
        IMediaControl_Stop(ctx->control);
        IMediaControl_Release(ctx->control);
    }

    if (ctx->media_event)
        IMediaEvent_Release(ctx->media_event);

    if (ctx->graph) {
        IEnumFilters *fenum;
        int r;
        r = IGraphBuilder_EnumFilters(ctx->graph, &fenum);
        if (r == S_OK) {
            IBaseFilter *f;
            IEnumFilters_Reset(fenum);
            while (IEnumFilters_Next(fenum, 1, &f, NULL) == S_OK) {
                if (IGraphBuilder_RemoveFilter(ctx->graph, f) == S_OK)
                    IEnumFilters_Reset(fenum); /* When a filter is removed,
                                                * the list must be reset. */
                IBaseFilter_Release(f);
            }
            IEnumFilters_Release(fenum);
        }
        IGraphBuilder_Release(ctx->graph);
    }

    if (ctx->capture_pin[VideoDevice])
        libAVPin_Release(ctx->capture_pin[VideoDevice]);
    if (ctx->capture_pin[AudioDevice])
        libAVPin_Release(ctx->capture_pin[AudioDevice]);
    if (ctx->capture_filter[VideoDevice])
        libAVFilter_Release(ctx->capture_filter[VideoDevice]);
    if (ctx->capture_filter[AudioDevice])
        libAVFilter_Release(ctx->capture_filter[AudioDevice]);

    if (ctx->device_pin[VideoDevice])
        IPin_Release(ctx->device_pin[VideoDevice]);
    if (ctx->device_pin[AudioDevice])
        IPin_Release(ctx->device_pin[AudioDevice]);
    if (ctx->device_filter[VideoDevice])
        IBaseFilter_Release(ctx->device_filter[VideoDevice]);
    if (ctx->device_filter[AudioDevice])
        IBaseFilter_Release(ctx->device_filter[AudioDevice]);

    if (ctx->device_name[0])
        av_freep(&ctx->device_name[0]);
    if (ctx->device_name[1])
        av_freep(&ctx->device_name[1]);

    if(ctx->mutex)
        CloseHandle(ctx->mutex);
    if(ctx->event[0])
        CloseHandle(ctx->event[0]);
    if(ctx->event[1])
        CloseHandle(ctx->event[1]);

    pktl = ctx->pktl;
    while (pktl) {
        AVPacketList *next = pktl->next;
        av_free_packet(&pktl->pkt);
        av_free(pktl);
        pktl = next;
    }

    CoUninitialize();

    return 0;
}

static char *dup_wchar_to_utf8(wchar_t *w)
{
    char *s = NULL;
    int l = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
    s = av_malloc(l);
    if (s)
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s, l, 0, 0);
    return s;
}

static int shall_we_drop(AVFormatContext *s, int index, enum dshowDeviceType devtype)
{
    struct dshow_ctx *ctx = s->priv_data;
    static const uint8_t dropscore[] = {62, 75, 87, 100};
    const int ndropscores = FF_ARRAY_ELEMS(dropscore);
    unsigned int buffer_fullness = (ctx->curbufsize[index]*100)/s->max_picture_buffer;
    const char *devtypename = (devtype == VideoDevice) ? "video" : "audio";

    if(dropscore[++ctx->video_frame_num%ndropscores] <= buffer_fullness) {
        av_log(s, AV_LOG_ERROR,
              "real-time buffer [%s] [%s input] too full or near too full (%d%% of size: %d [rtbufsize parameter])! frame dropped!\n",
              ctx->device_name[devtype], devtypename, buffer_fullness, s->max_picture_buffer);
        return 1;
    }

    return 0;
}

static void
callback(void *priv_data, int index, uint8_t *buf, int buf_size, int64_t time, enum dshowDeviceType devtype)
{
    AVFormatContext *s = priv_data;
    struct dshow_ctx *ctx = s->priv_data;
    AVPacketList **ppktl, *pktl_next;

//    dump_videohdr(s, vdhdr);

    WaitForSingleObject(ctx->mutex, INFINITE);

    if(shall_we_drop(s, index, devtype))
        goto fail;

    pktl_next = av_mallocz(sizeof(AVPacketList));
    if(!pktl_next)
        goto fail;

    if(av_new_packet(&pktl_next->pkt, buf_size) < 0) {
        av_free(pktl_next);
        goto fail;
    }

    pktl_next->pkt.stream_index = index;
    pktl_next->pkt.pts = time;
    memcpy(pktl_next->pkt.data, buf, buf_size);

    for(ppktl = &ctx->pktl ; *ppktl ; ppktl = &(*ppktl)->next);
    *ppktl = pktl_next;
    ctx->curbufsize[index] += buf_size;

    SetEvent(ctx->event[1]);
    ReleaseMutex(ctx->mutex);

    return;
fail:
    ReleaseMutex(ctx->mutex);
    return;
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

/**
 * Cycle through available devices using the device enumerator devenum,
 * retrieve the device with type specified by devtype and return the
 * pointer to the object found in *pfilter.
 * If pfilter is NULL, list all device names.
 */
static int
dshow_cycle_devices(AVFormatContext *avctx, ICreateDevEnum *devenum,
                    enum dshowDeviceType devtype, enum dshowSourceFilterType sourcetype, IBaseFilter **pfilter)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IBaseFilter *device_filter = NULL;
    IEnumMoniker *classenum = NULL;
    IMoniker *m = NULL;
    const char *device_name = ctx->device_name[devtype];
    int skip = (devtype == VideoDevice) ? ctx->video_device_number
                                        : ctx->audio_device_number;
    int r;

    const GUID *device_guid[2] = { &CLSID_VideoInputDeviceCategory,
                                   &CLSID_AudioInputDeviceCategory };
    const char *devtypename = (devtype == VideoDevice) ? "video" : "audio only";
    const char *sourcetypename = (sourcetype == VideoSourceDevice) ? "video" : "audio";

    r = ICreateDevEnum_CreateClassEnumerator(devenum, device_guid[sourcetype],
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

/**
 * Cycle through available formats using the specified pin,
 * try to set parameters specified through AVOptions and if successful
 * return 1 in *pformat_set.
 * If pformat_set is NULL, list all pin capabilities.
 */
static void
dshow_cycle_formats(AVFormatContext *avctx, enum dshowDeviceType devtype,
                    IPin *pin, int *pformat_set)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IAMStreamConfig *config = NULL;
    AM_MEDIA_TYPE *type = NULL;
    int format_set = 0;
    void *caps = NULL;
    int i, n, size, r;

    if (IPin_QueryInterface(pin, &IID_IAMStreamConfig, (void **) &config) != S_OK)
        return;
    if (IAMStreamConfig_GetNumberOfCapabilities(config, &n, &size) != S_OK)
        goto end;

    caps = av_malloc(size);
    if (!caps)
        goto end;

    for (i = 0; i < n && !format_set; i++) {
        r = IAMStreamConfig_GetStreamCaps(config, i, &type, (void *) caps);
        if (r != S_OK)
            goto next;
#if DSHOWDEBUG
        ff_print_AM_MEDIA_TYPE(type);
#endif
        if (devtype == VideoDevice) {
            VIDEO_STREAM_CONFIG_CAPS *vcaps = caps;
            BITMAPINFOHEADER *bih;
            int64_t *fr;
            const AVCodecTag *const tags[] = { avformat_get_riff_video_tags(), NULL };
#if DSHOWDEBUG
            ff_print_VIDEO_STREAM_CONFIG_CAPS(vcaps);
#endif
            if (IsEqualGUID(&type->formattype, &FORMAT_VideoInfo)) {
                VIDEOINFOHEADER *v = (void *) type->pbFormat;
                fr = &v->AvgTimePerFrame;
                bih = &v->bmiHeader;
            } else if (IsEqualGUID(&type->formattype, &FORMAT_VideoInfo2)) {
                VIDEOINFOHEADER2 *v = (void *) type->pbFormat;
                fr = &v->AvgTimePerFrame;
                bih = &v->bmiHeader;
            } else {
                goto next;
            }
            if (!pformat_set) {
                enum AVPixelFormat pix_fmt = dshow_pixfmt(bih->biCompression, bih->biBitCount);
                if (pix_fmt == AV_PIX_FMT_NONE) {
                    enum AVCodecID codec_id = av_codec_get_id(tags, bih->biCompression);
                    AVCodec *codec = avcodec_find_decoder(codec_id);
                    if (codec_id == AV_CODEC_ID_NONE || !codec) {
                        av_log(avctx, AV_LOG_INFO, "  unknown compression type 0x%X", (int) bih->biCompression);
                    } else {
                        av_log(avctx, AV_LOG_INFO, "  vcodec=%s", codec->name);
                    }
                } else {
                    av_log(avctx, AV_LOG_INFO, "  pixel_format=%s", av_get_pix_fmt_name(pix_fmt));
                }
                av_log(avctx, AV_LOG_INFO, "  min s=%ldx%ld fps=%g max s=%ldx%ld fps=%g\n",
                       vcaps->MinOutputSize.cx, vcaps->MinOutputSize.cy,
                       1e7 / vcaps->MaxFrameInterval,
                       vcaps->MaxOutputSize.cx, vcaps->MaxOutputSize.cy,
                       1e7 / vcaps->MinFrameInterval);
                continue;
            }
            if (ctx->video_codec_id != AV_CODEC_ID_RAWVIDEO) {
                if (ctx->video_codec_id != av_codec_get_id(tags, bih->biCompression))
                    goto next;
            }
            if (ctx->pixel_format != AV_PIX_FMT_NONE &&
                ctx->pixel_format != dshow_pixfmt(bih->biCompression, bih->biBitCount)) {
                goto next;
            }
            if (ctx->framerate) {
                int64_t framerate = ((int64_t) ctx->requested_framerate.den*10000000)
                                            /  ctx->requested_framerate.num;
                if (framerate > vcaps->MaxFrameInterval ||
                    framerate < vcaps->MinFrameInterval)
                    goto next;
                *fr = framerate;
            }
            if (ctx->requested_width && ctx->requested_height) {
                if (ctx->requested_width  > vcaps->MaxOutputSize.cx ||
                    ctx->requested_width  < vcaps->MinOutputSize.cx ||
                    ctx->requested_height > vcaps->MaxOutputSize.cy ||
                    ctx->requested_height < vcaps->MinOutputSize.cy)
                    goto next;
                bih->biWidth  = ctx->requested_width;
                bih->biHeight = ctx->requested_height;
            }
        } else {
            AUDIO_STREAM_CONFIG_CAPS *acaps = caps;
            WAVEFORMATEX *fx;
#if DSHOWDEBUG
            ff_print_AUDIO_STREAM_CONFIG_CAPS(acaps);
#endif
            if (IsEqualGUID(&type->formattype, &FORMAT_WaveFormatEx)) {
                fx = (void *) type->pbFormat;
            } else {
                goto next;
            }
            if (!pformat_set) {
                av_log(avctx, AV_LOG_INFO, "  min ch=%lu bits=%lu rate=%6lu max ch=%lu bits=%lu rate=%6lu\n",
                       acaps->MinimumChannels, acaps->MinimumBitsPerSample, acaps->MinimumSampleFrequency,
                       acaps->MaximumChannels, acaps->MaximumBitsPerSample, acaps->MaximumSampleFrequency);
                continue;
            }
            if (ctx->sample_rate) {
                if (ctx->sample_rate > acaps->MaximumSampleFrequency ||
                    ctx->sample_rate < acaps->MinimumSampleFrequency)
                    goto next;
                fx->nSamplesPerSec = ctx->sample_rate;
            }
            if (ctx->sample_size) {
                if (ctx->sample_size > acaps->MaximumBitsPerSample ||
                    ctx->sample_size < acaps->MinimumBitsPerSample)
                    goto next;
                fx->wBitsPerSample = ctx->sample_size;
            }
            if (ctx->channels) {
                if (ctx->channels > acaps->MaximumChannels ||
                    ctx->channels < acaps->MinimumChannels)
                    goto next;
                fx->nChannels = ctx->channels;
            }
        }
        if (IAMStreamConfig_SetFormat(config, type) != S_OK)
            goto next;
        format_set = 1;
next:
        if (type->pbFormat)
            CoTaskMemFree(type->pbFormat);
        CoTaskMemFree(type);
    }
end:
    IAMStreamConfig_Release(config);
    av_free(caps);
    if (pformat_set)
        *pformat_set = format_set;
}

/**
 * Set audio device buffer size in milliseconds (which can directly impact
 * latency, depending on the device).
 */
static int
dshow_set_audio_buffer_size(AVFormatContext *avctx, IPin *pin)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IAMBufferNegotiation *buffer_negotiation = NULL;
    ALLOCATOR_PROPERTIES props = { -1, -1, -1, -1 };
    IAMStreamConfig *config = NULL;
    AM_MEDIA_TYPE *type = NULL;
    int ret = AVERROR(EIO);

    if (IPin_QueryInterface(pin, &IID_IAMStreamConfig, (void **) &config) != S_OK)
        goto end;
    if (IAMStreamConfig_GetFormat(config, &type) != S_OK)
        goto end;
    if (!IsEqualGUID(&type->formattype, &FORMAT_WaveFormatEx))
        goto end;

    props.cbBuffer = (((WAVEFORMATEX *) type->pbFormat)->nAvgBytesPerSec)
                   * ctx->audio_buffer_size / 1000;

    if (IPin_QueryInterface(pin, &IID_IAMBufferNegotiation, (void **) &buffer_negotiation) != S_OK)
        goto end;
    if (IAMBufferNegotiation_SuggestAllocatorProperties(buffer_negotiation, &props) != S_OK)
        goto end;

    ret = 0;

end:
    if (buffer_negotiation)
        IAMBufferNegotiation_Release(buffer_negotiation);
    if (type) {
        if (type->pbFormat)
            CoTaskMemFree(type->pbFormat);
        CoTaskMemFree(type);
    }
    if (config)
        IAMStreamConfig_Release(config);

    return ret;
}

/**
 * Pops up a user dialog allowing them to adjust properties for the given filter, if possible.
 */
void
dshow_show_filter_properties(IBaseFilter *device_filter, AVFormatContext *avctx) {
    ISpecifyPropertyPages *property_pages = NULL;
    IUnknown *device_filter_iunknown = NULL;
    HRESULT hr;
    FILTER_INFO filter_info = {0}; /* a warning on this line is false positive GCC bug 53119 AFAICT */
    CAUUID ca_guid = {0};

    hr  = IBaseFilter_QueryInterface(device_filter, &IID_ISpecifyPropertyPages, (void **)&property_pages);
    if (hr != S_OK) {
        av_log(avctx, AV_LOG_WARNING, "requested filter does not have a property page to show");
        goto end;
    }
    hr = IBaseFilter_QueryFilterInfo(device_filter, &filter_info);
    if (hr != S_OK) {
        goto fail;
    }
    hr = IBaseFilter_QueryInterface(device_filter, &IID_IUnknown, (void **)&device_filter_iunknown);
    if (hr != S_OK) {
        goto fail;
    }
    hr = ISpecifyPropertyPages_GetPages(property_pages, &ca_guid);
    if (hr != S_OK) {
        goto fail;
    }
    hr = OleCreatePropertyFrame(NULL, 0, 0, filter_info.achName, 1, &device_filter_iunknown, ca_guid.cElems,
        ca_guid.pElems, 0, 0, NULL);
    if (hr != S_OK) {
        goto fail;
    }
    goto end;
fail:
    av_log(avctx, AV_LOG_ERROR, "Failure showing property pages for filter");
end:
    if (property_pages)
        ISpecifyPropertyPages_Release(property_pages);
    if (device_filter_iunknown)
        IUnknown_Release(device_filter_iunknown);
    if (filter_info.pGraph)
        IFilterGraph_Release(filter_info.pGraph);
    if (ca_guid.pElems)
        CoTaskMemFree(ca_guid.pElems);
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
    r = dshow_lookup_pin(avctx, source, PINDIR_OUTPUT, &pin_out, src_pin_name, "src");
    if (r != S_OK) {
        return AVERROR(EIO);
    }
    r = dshow_lookup_pin(avctx, destination, PINDIR_INPUT, &pin_in, dest_pin_name, "dest");
    if (r != S_OK) {
        return AVERROR(EIO);
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
            return AVERROR(EIO);
        }
    }

    return 0;
}

/**
 * Cycle through available pins using the device_filter device, of type
 * devtype, retrieve the first output pin and return the pointer to the
 * object found in *ppin.
 * If ppin is NULL, cycle through all pins listing audio/video capabilities.
 */
static int
dshow_cycle_pins(AVFormatContext *avctx, enum dshowDeviceType devtype,
                 enum dshowSourceFilterType sourcetype, IBaseFilter *device_filter, IPin **ppin)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IEnumPins *pins = 0;
    IPin *device_pin = NULL;
    IPin *pin;
    int r;

    const GUID *mediatype[2] = { &MEDIATYPE_Video, &MEDIATYPE_Audio };
    const char *devtypename = (devtype == VideoDevice) ? "video" : "audio only";
    const char *sourcetypename = (sourcetype == VideoSourceDevice) ? "video" : "audio";

    int set_format = (devtype == VideoDevice && (ctx->framerate ||
                                                (ctx->requested_width && ctx->requested_height) ||
                                                 ctx->pixel_format != AV_PIX_FMT_NONE ||
                                                 ctx->video_codec_id != AV_CODEC_ID_RAWVIDEO))
                  || (devtype == AudioDevice && (ctx->channels || ctx->sample_rate));
    int format_set = 0;
    int should_show_properties = (devtype == VideoDevice) ? ctx->show_video_device_dialog : ctx->show_audio_device_dialog;

    if (should_show_properties)
        dshow_show_filter_properties(device_filter, avctx);

    r = IBaseFilter_EnumPins(device_filter, &pins);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not enumerate pins.\n");
        return AVERROR(EIO);
    }

    if (!ppin) {
        av_log(avctx, AV_LOG_INFO, "DirectShow %s device options (from %s devices)\n",
               devtypename, sourcetypename);
    }

    while (!device_pin && IEnumPins_Next(pins, 1, &pin, NULL) == S_OK) {
        IKsPropertySet *p = NULL;
        IEnumMediaTypes *types = NULL;
        PIN_INFO info = {0};
        AM_MEDIA_TYPE *type;
        GUID category;
        DWORD r2;
        char *name_buf = NULL;
        wchar_t *pin_id = NULL;
        char *pin_buf = NULL;
        char *desired_pin_name = devtype == VideoDevice ? ctx->video_pin_name : ctx->audio_pin_name;

        IPin_QueryPinInfo(pin, &info);
        IBaseFilter_Release(info.pFilter);

        if (info.dir != PINDIR_OUTPUT)
            goto next;
        if (IPin_QueryInterface(pin, &IID_IKsPropertySet, (void **) &p) != S_OK)
            goto next;
        if (IKsPropertySet_Get(p, &AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                               NULL, 0, &category, sizeof(GUID), &r2) != S_OK)
            goto next;
        if (!IsEqualGUID(&category, &PIN_CATEGORY_CAPTURE))
            goto next;
        name_buf = dup_wchar_to_utf8(info.achName);

        r = IPin_QueryId(pin, &pin_id);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not query pin id\n");
            return AVERROR(EIO);
        }
        pin_buf = dup_wchar_to_utf8(pin_id);

        if (!ppin) {
            av_log(avctx, AV_LOG_INFO, " Pin \"%s\" (alternative pin name \"%s\")\n", name_buf, pin_buf);
            dshow_cycle_formats(avctx, devtype, pin, NULL);
            goto next;
        }

        if (desired_pin_name) {
            if(strcmp(name_buf, desired_pin_name) && strcmp(pin_buf, desired_pin_name)) {
                av_log(avctx, AV_LOG_DEBUG, "skipping pin \"%s\" (\"%s\") != requested \"%s\"\n",
                    name_buf, pin_buf, desired_pin_name);
                goto next;
            }
        }

        if (set_format) {
            dshow_cycle_formats(avctx, devtype, pin, &format_set);
            if (!format_set) {
                goto next;
            }
        }
        if (devtype == AudioDevice && ctx->audio_buffer_size) {
            if (dshow_set_audio_buffer_size(avctx, pin) < 0) {
                av_log(avctx, AV_LOG_ERROR, "unable to set audio buffer size %d to pin, using pin anyway...", ctx->audio_buffer_size);
            }
        }

        if (IPin_EnumMediaTypes(pin, &types) != S_OK)
            goto next;

        IEnumMediaTypes_Reset(types);
        /* in case format_set was not called, just verify the majortype */
        while (!device_pin && IEnumMediaTypes_Next(types, 1, &type, NULL) == S_OK) {
            if (IsEqualGUID(&type->majortype, mediatype[devtype])) {
                device_pin = pin;
                av_log(avctx, AV_LOG_DEBUG, "Selecting pin %s on %s\n", name_buf, devtypename);
                goto next;
            }
            CoTaskMemFree(type);
        }

next:
        if (types)
            IEnumMediaTypes_Release(types);
        if (p)
            IKsPropertySet_Release(p);
        if (device_pin != pin)
            IPin_Release(pin);
        av_free(name_buf);
        av_free(pin_buf);
        if (pin_id)
            CoTaskMemFree(pin_id);
    }

    IEnumPins_Release(pins);

    if (ppin) {
        if (set_format && !format_set) {
            av_log(avctx, AV_LOG_ERROR, "Could not set %s options\n", devtypename);
            return AVERROR(EIO);
        }
        if (!device_pin) {
            av_log(avctx, AV_LOG_ERROR,
                "Could not find output pin from %s capture device.\n", devtypename);
            return AVERROR(EIO);
        }
        *ppin = device_pin;
    }

    return 0;
}

/**
 * List options for device with type devtype, source filter type sourcetype
 *
 * @param devenum device enumerator used for accessing the device
 */
static int
dshow_list_device_options(AVFormatContext *avctx, ICreateDevEnum *devenum,
                          enum dshowDeviceType devtype, enum dshowSourceFilterType sourcetype)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IBaseFilter *device_filter = NULL;
    int r;

    if ((r = dshow_cycle_devices(avctx, devenum, devtype, sourcetype, &device_filter)) < 0)
        return r;
    ctx->device_filter[devtype] = device_filter;
    if ((r = dshow_cycle_pins(avctx, devtype, sourcetype, device_filter, NULL)) < 0)
        return r;

    return 0;
}

static int
dshow_open_device(AVFormatContext *avctx, ICreateDevEnum *devenum,
                  enum dshowDeviceType devtype, enum dshowSourceFilterType sourcetype)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IBaseFilter *device_filter = NULL;
    IGraphBuilder *graph = ctx->graph;
    IPin *device_pin = NULL;
    libAVPin *capture_pin = NULL;
    libAVFilter *capture_filter = NULL;
    ICaptureGraphBuilder2 *graph_builder2 = NULL;
    int ret = AVERROR(EIO);
    int r;
    IStream *ifile_stream = NULL;
    IStream *ofile_stream = NULL;
    IPersistStream *pers_stream = NULL;

    const wchar_t *filter_name[2] = { L"Audio capture filter", L"Video capture filter" };


    if ( ((ctx->audio_filter_load_file) && (strlen(ctx->audio_filter_load_file)>0) && (sourcetype == AudioSourceDevice)) ||
            ((ctx->video_filter_load_file) && (strlen(ctx->video_filter_load_file)>0) && (sourcetype == VideoSourceDevice)) ) {
        HRESULT hr;
        char *filename = NULL;

        if (sourcetype == AudioSourceDevice)
            filename = ctx->audio_filter_load_file;
        else
            filename = ctx->video_filter_load_file;

        hr = SHCreateStreamOnFile ((LPCSTR) filename, STGM_READ, &ifile_stream);
        if (S_OK != hr) {
            av_log(avctx, AV_LOG_ERROR, "Could not open capture filter description file.\n");
            goto error;
        }

        hr = OleLoadFromStream(ifile_stream, &IID_IBaseFilter, (void **) &device_filter);
        if (hr != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not load capture filter from file.\n");
            goto error;
        }

        if (sourcetype == AudioSourceDevice)
            av_log(avctx, AV_LOG_INFO, "Audio-");
        else
            av_log(avctx, AV_LOG_INFO, "Video-");
        av_log(avctx, AV_LOG_INFO, "Capture filter loaded successfully from file \"%s\".\n", filename);
    } else {

        if ((r = dshow_cycle_devices(avctx, devenum, devtype, sourcetype, &device_filter)) < 0) {
            ret = r;
            goto error;
        }
    }

    ctx->device_filter [devtype] = device_filter;

    r = IGraphBuilder_AddFilter(graph, device_filter, NULL);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not add device filter to graph.\n");
        goto error;
    }

    if ((r = dshow_cycle_pins(avctx, devtype, sourcetype, device_filter, &device_pin)) < 0) {
        ret = r;
        goto error;
    }

    ctx->device_pin[devtype] = device_pin;

    capture_filter = libAVFilter_Create(avctx, callback, devtype);
    if (!capture_filter) {
        av_log(avctx, AV_LOG_ERROR, "Could not create grabber filter.\n");
        goto error;
    }
    ctx->capture_filter[devtype] = capture_filter;

    if ( ((ctx->audio_filter_save_file) && (strlen(ctx->audio_filter_save_file)>0) && (sourcetype == AudioSourceDevice)) ||
            ((ctx->video_filter_save_file) && (strlen(ctx->video_filter_save_file)>0) && (sourcetype == VideoSourceDevice)) ) {

        HRESULT hr;
        char *filename = NULL;

        if (sourcetype == AudioSourceDevice)
            filename = ctx->audio_filter_save_file;
        else
            filename = ctx->video_filter_save_file;

        hr = SHCreateStreamOnFile ((LPCSTR) filename, STGM_CREATE | STGM_READWRITE, &ofile_stream);
        if (S_OK != hr) {
            av_log(avctx, AV_LOG_ERROR, "Could not create capture filter description file.\n");
            goto error;
        }

        hr  = IBaseFilter_QueryInterface(device_filter, &IID_IPersistStream, (void **) &pers_stream);
        if (hr != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Query for IPersistStream failed.\n");
            goto error;
        }

        hr = OleSaveToStream(pers_stream, ofile_stream);
        if (hr != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not save capture filter \n");
            goto error;
        }

        hr = IStream_Commit(ofile_stream, STGC_DEFAULT);
        if (S_OK != hr) {
            av_log(avctx, AV_LOG_ERROR, "Could not commit capture filter data to file.\n");
            goto error;
        }

        if (sourcetype == AudioSourceDevice)
            av_log(avctx, AV_LOG_INFO, "Audio-");
        else
            av_log(avctx, AV_LOG_INFO, "Video-");
        av_log(avctx, AV_LOG_INFO, "Capture filter saved successfully to file \"%s\".\n", filename);
    }

    r = IGraphBuilder_AddFilter(graph, (IBaseFilter *) capture_filter,
                                filter_name[devtype]);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not add capture filter to graph\n");
        goto error;
    }

    libAVPin_AddRef(capture_filter->pin);
    capture_pin = capture_filter->pin;
    ctx->capture_pin[devtype] = capture_pin;

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

    r = ICaptureGraphBuilder2_RenderStream(graph_builder2, NULL, NULL, (IUnknown *) device_pin, NULL /* no intermediate filter */,
        (IBaseFilter *) capture_filter); /* connect pins, optionally insert intermediate filters like crossbar if necessary */

    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not RenderStream to connect pins\n");
        goto error;
    }

    r = dshow_try_setup_crossbar_options(graph_builder2, device_filter, devtype, avctx);

    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not setup CrossBar\n");
        goto error;
    }

    ret = 0;

error:
    if (graph_builder2 != NULL)
        ICaptureGraphBuilder2_Release(graph_builder2);

    if (pers_stream)
        IPersistStream_Release(pers_stream);

    if (ifile_stream)
        IStream_Release(ifile_stream);

    if (ofile_stream)
        IStream_Release(ofile_stream);

    return ret;
}

static enum AVCodecID waveform_codec_id(enum AVSampleFormat sample_fmt)
{
    switch (sample_fmt) {
    case AV_SAMPLE_FMT_U8:  return AV_CODEC_ID_PCM_U8;
    case AV_SAMPLE_FMT_S16: return AV_CODEC_ID_PCM_S16LE;
    case AV_SAMPLE_FMT_S32: return AV_CODEC_ID_PCM_S32LE;
    default:                return AV_CODEC_ID_NONE; /* Should never happen. */
    }
}

static enum AVSampleFormat sample_fmt_bits_per_sample(int bits)
{
    switch (bits) {
    case 8:  return AV_SAMPLE_FMT_U8;
    case 16: return AV_SAMPLE_FMT_S16;
    case 32: return AV_SAMPLE_FMT_S32;
    default: return AV_SAMPLE_FMT_NONE; /* Should never happen. */
    }
}

static int
dshow_add_device(AVFormatContext *avctx,
                 enum dshowDeviceType devtype)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    AM_MEDIA_TYPE type;
    AVCodecContext *codec;
    AVStream *st;
    int ret = AVERROR(EIO);
    int is_mpeg = 0;

    st = avformat_new_stream(avctx, NULL);
    if (!st) {
        ret = AVERROR(ENOMEM);
        goto error;
    }
    st->id = devtype;

    ctx->capture_filter[devtype]->stream_index = st->index;

    libAVPin_ConnectionMediaType(ctx->capture_pin[devtype], &type);
    printf("here is it\n");
    ff_print_AM_MEDIA_TYPE(&type);

    codec = st->codec;
    if (devtype == VideoDevice) {
        BITMAPINFOHEADER *bih = NULL;
        AVRational time_base;
        if (IsEqualGUID(&type.formattype, &FORMAT_VideoInfo)) {
            VIDEOINFOHEADER *v = (void *) type.pbFormat;
            time_base = (AVRational) { v->AvgTimePerFrame, 10000000 };
            bih = &v->bmiHeader;
        } else if (IsEqualGUID(&type.formattype, &FORMAT_VideoInfo2)) {
            VIDEOINFOHEADER2 *v = (void *) type.pbFormat;
            time_base = (AVRational) { v->AvgTimePerFrame, 10000000 };
            bih = &v->bmiHeader;
        } else if (IsEqualGUID(&type.formattype, &FORMAT_MPEG2_VIDEO) && type.cbFormat >= sizeof(MPEG2VIDEOINFO)) {
          // get here if allowed for in dshow_pin.c
          MPEG2VIDEOINFO *mpeg_video_info = (void *) type.pbFormat;
          VIDEOINFOHEADER2 *v = (void *) &mpeg_video_info->hdr;
          time_base = (AVRational) { v->AvgTimePerFrame, 10000000 };
          bih = &v->bmiHeader;
          //AVMEDIA_TYPE_VIDEO AV_CODEC_ID_MPEG2VIDEO
          is_mpeg = 1; // NB this has biWidth set but its actually "wrong" or "could be overriden" or the like <sigh>
        } else if (IsEqualGUID(&type.subtype, &KSDATAFORMAT_SUBTYPE_BDA_MPEG2_TRANSPORT_LOCAL)) {
            // get here if using infinite tee
            avpriv_set_pts_info(st, 60, 1, 27000000);
            codec->codec_id = AV_CODEC_ID_MPEG2TS; // the magic one it doesn't understand
            codec->codec_type = AVMEDIA_TYPE_DATA;
            goto done;
        } 
        if (!bih) {
            av_log(avctx, AV_LOG_ERROR, "Could not get media type.\n");
            ff_printGUID(&type.formattype);
            goto error;
        }

        codec->time_base  = time_base;
        codec->codec_type = AVMEDIA_TYPE_VIDEO;
        codec->width      = bih->biWidth;
        codec->height     = bih->biHeight;
        codec->codec_tag  = bih->biCompression;
        codec->pix_fmt    = dshow_pixfmt(bih->biCompression, bih->biBitCount);
        if (bih->biCompression == MKTAG('H', 'D', 'Y', 'C')) {
            av_log(avctx, AV_LOG_DEBUG, "attempt to use full range for HDYC...\n");
            codec->color_range = AVCOL_RANGE_MPEG; // just in case it needs this...
        }
        if (codec->pix_fmt == AV_PIX_FMT_NONE) {
             const AVCodecTag *const tags[] = { avformat_get_riff_video_tags(), NULL };
             if (is_mpeg) {
               codec->codec_id = AV_CODEC_ID_MPEG2VIDEO;
             } else {
               codec->codec_id = av_codec_get_id(tags, bih->biCompression);
               if (codec->codec_id == AV_CODEC_ID_NONE) {
                   av_log(avctx, AV_LOG_ERROR, "Unknown compression type. "
                                  "Please report type 0x%X.\n", (int) bih->biCompression);
                   return AVERROR_PATCHWELCOME;
              }
            }
            codec->bits_per_coded_sample = bih->biBitCount;
        } else {
            codec->codec_id = AV_CODEC_ID_RAWVIDEO;
            if (bih->biCompression == BI_RGB || bih->biCompression == BI_BITFIELDS) {
                codec->bits_per_coded_sample = bih->biBitCount;
                codec->extradata = av_malloc(9 + AV_INPUT_BUFFER_PADDING_SIZE);
                if (codec->extradata) {
                    codec->extradata_size = 9;
                    memcpy(codec->extradata, "BottomUp", 9);
                }
            }
        }
    } else {
        WAVEFORMATEX *fx = NULL;

        if (IsEqualGUID(&type.formattype, &FORMAT_WaveFormatEx)) {
            fx = (void *) type.pbFormat;
        }
        if (!fx) {
            av_log(avctx, AV_LOG_ERROR, "Could not get media type.\n");
            goto error;
        }

        codec->codec_type  = AVMEDIA_TYPE_AUDIO;
        codec->sample_fmt  = sample_fmt_bits_per_sample(fx->wBitsPerSample);
        codec->codec_id    = waveform_codec_id(codec->sample_fmt);
        codec->sample_rate = fx->nSamplesPerSec;
        codec->channels    = fx->nChannels;
    }

    avpriv_set_pts_info(st, 64, 1, 10000000);
done:
    ret = 0;

error:
    return ret;
}

static int parse_device_name(AVFormatContext *avctx)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    char **device_name = ctx->device_name; // device_name array
    char *name = av_strdup(avctx->filename);
    char *tmp = name;
    int ret = 1;
    char *type;

    while ((type = strtok(tmp, "="))) {
        char *token = strtok(NULL, ":");
        tmp = NULL;

        if        (!strcmp(type, "video")) {
            device_name[0] = token;
        } else if (!strcmp(type, "audio")) {
            device_name[1] = token;
        } else {
            device_name[0] = NULL;
            device_name[1] = NULL;
            break;
        }
    }

    if (!device_name[0] && !device_name[1]) {
        ret = 0;
    } else {
        if (device_name[0])
            device_name[0] = av_strdup(device_name[0]);
        if (device_name[1])
            device_name[1] = av_strdup(device_name[1]);
    }
    av_log(avctx, AV_LOG_DEBUG, "parsed names as video=[%s] audio=[%s]\n", device_name[0], device_name[1]);

    av_free(name);
    return ret;
}

static int dshow_read_header(AVFormatContext *avctx)
{
    struct dshow_ctx *ctx = avctx->priv_data;
    IGraphBuilder *graph = NULL;
    ICreateDevEnum *devenum = NULL;
    IMediaControl *control = NULL;
    IMediaEvent *media_event = NULL;
    HANDLE media_event_handle;
    HANDLE proc;
    int ret = AVERROR(EIO);
    int r;

    CoInitialize(0);

    if (!ctx->list_devices && !parse_device_name(avctx)) {
        av_log(avctx, AV_LOG_ERROR, "Malformed dshow input string.\n");
        goto error;
    }

    if (ctx->dtv){
        IBaseFilter *bda_net_provider = NULL;
        IBaseFilter *bda_source_device = NULL;
        IBaseFilter *bda_receiver_device = NULL;
        IBaseFilter *bda_filter_supplying_mpeg = NULL;
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
        int use_infinite_tee_ts_stream = 0;

        const wchar_t *filter_name[2] = { L"Audio capture filter", L"Video capture filter" };

        ///create graph

        r = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IGraphBuilder, (void **) &graph);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create capture graph.\n");
            goto error;
        }
        ctx->graph = graph;

        r = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                             &IID_ICreateDevEnum, (void **) &devenum);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not enumerate system devices.\n");
            goto error;
        }

        ///add microsoft network provider

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


        ///add dtv input
        if (ctx->list_devices) {
            av_log(avctx, AV_LOG_INFO, "BDA tuners:\n");
            dshow_cycle_dtv_devices(avctx, NetworkTuner, devenum, NULL);
            av_log(avctx, AV_LOG_INFO, "BDA receivers:\n");
            dshow_cycle_dtv_devices(avctx, ReceiverComponent, devenum, NULL);
            goto error;
        }


        //////////////////////////////////
        /////////////setup tuner


        ///create tune request

        r = IBaseFilter_QueryInterface(bda_net_provider, &IID_IScanningTuner, (void**) &scanning_tuner);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not query scanning tuner.\n");
            goto error;
        }


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

            SysFreeString(bstr_name);

            r = ITuningSpace_get_FriendlyName(tuning_space, &bstr_name);
            if(r != S_OK) {
                /* should never fail on a good tuning space */
                av_log(avctx, AV_LOG_ERROR, "Cannot get UniqueName for Tuning Space: r=0x%8x\n", r );
                goto error;
            }

            psz_bstr_name = dup_wchar_to_utf8(bstr_name);
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

        av_log(avctx, AV_LOG_INFO, "Using def locator\n");

        r = ITuningSpace_get_DefaultLocator(tuning_space, &def_locator);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not get default locator\n");
            goto error;
        }

        av_log(avctx, AV_LOG_INFO, "create tune request\n");


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

        av_log(avctx, AV_LOG_INFO, "Def loator pre validate\n");

        r = IScanningTuner_Validate(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Error validating dvb tune request: r=0x%8x\n", r);
            goto error;
        }

        r = IScanningTuner_put_TuneRequest(scanning_tuner, tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not set dvb tune request\n");
            goto error;
        }


        ///////////////
        /////////////////////////////////////


        ///---add network tuner
        if ((r = dshow_cycle_dtv_devices(avctx, NetworkTuner, devenum, &bda_source_device)) < 0) {
            ret = r;
            av_log(avctx, AV_LOG_ERROR, "Could not find BDA tuner.\n");
            goto error;
        }

        bda_filter_supplying_mpeg =  bda_source_device;

        r = IGraphBuilder_AddFilter(graph, bda_source_device, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not add BDA tuner to graph.\n");
            goto error;
        }


        av_log(avctx, AV_LOG_INFO, "BDA tuner added\n");

        ///connect msnetprovider and dtv

        r = dshow_connect_bda_pins(avctx, bda_net_provider, NULL, bda_source_device, NULL, NULL, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not connect network provider to tuner. Trying generic Network Provider.\n");
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

            av_log(avctx, AV_LOG_INFO, "Generic Network Provider added\n");

        }

        ///---add receive component if requested
        if (ctx->receiver_component) {

            // find right named device
            if ((r = dshow_cycle_dtv_devices(avctx, ReceiverComponent, devenum, &bda_receiver_device)) < 0) {
                ret = r;
                goto error;
            }

            bda_filter_supplying_mpeg = bda_receiver_device; 

            r = IGraphBuilder_AddFilter(graph, bda_receiver_device, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not add BDA receiver to graph.\n");
                goto error;
            }

            av_log(avctx, AV_LOG_INFO, "BDA Receiver Component added\n");

            ///connect tuner to receiver

            r = dshow_connect_bda_pins(avctx, bda_source_device, NULL, bda_receiver_device, NULL, NULL, NULL);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not connect tuner to receiver component.\n");
                goto error;
            }
        }


        // create infinite tee so we can just grab the MPEG stream -> ffmpeg
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
        av_log(avctx, AV_LOG_ERROR, "success infinite tee to graph.\n");


        r = dshow_connect_bda_pins(avctx, bda_filter_supplying_mpeg, NULL, bda_infinite_tee, NULL, NULL, NULL);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not connect bda supplier to infinite tee.\n");
            goto error;
        }


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
          r = dshow_connect_bda_pins(avctx, bda_infinite_tee, NULL, bda_mpeg2_demux, NULL, NULL, NULL); // TODO fix me! 003 also '3"
        else
          r = dshow_connect_bda_pins(avctx, bda_infinite_tee, NULL, bda_mpeg2_demux, NULL, &bda_mpeg_video_pin, "3"); // TODO fix me! 003 also
        if (r != S_OK) {
            goto error;
        }


        // after this point the infinite tee will now have an "Output2" named pin
        if (use_infinite_tee_ts_stream) {
          r = dshow_lookup_pin(avctx, bda_infinite_tee, PINDIR_OUTPUT, &bda_mpeg_video_pin, "Output2", "split mpeg tee pin");
          if (r != S_OK) {
            goto error;
          }
        }

        //add DBA MPEG2 Transport information filter

        r = CoCreateInstance(&CLSID_BDA_MPEG2_Transport_Informatiuon_Filter, NULL, CLSCTX_INPROC_SERVER,
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
            av_log(avctx, AV_LOG_ERROR, "Could not connect mpeg2 demux to mpeg2 transport information filter.\n");
            goto error;
        }


        /////////////////////////////////
        ///////////////DTV tuning

        av_log(avctx, AV_LOG_INFO, "Using DBA tuning stuff\n");

        r = IScanningTuner_get_TuneRequest(scanning_tuner, &tune_request);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not get tune request -- Trying to create one\n");
            r = ITuningSpace_CreateTuneRequest(tuning_space, &tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not create tune request\n");
                goto error;
            }
        }


        //// if tuningspace == DVB
        if ((ctx->dtv > 0) && (ctx->dtv < 4)) {

            r = ITuneRequest_QueryInterface(tune_request, &IID_IDVBTuneRequest, (void **) &dvb_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not query DVBTuneRequest.\n");
                goto error;
            }

            av_log(avctx, AV_LOG_INFO, "DVBTuneRequest acquired.\n");

            IDVBTuneRequest_put_ONID(dvb_tune_request, -1 );
            IDVBTuneRequest_put_SID(dvb_tune_request, -1 );
            IDVBTuneRequest_put_TSID(dvb_tune_request, -1 );

            //// if tuningspace  == DVB-C
            if (ctx->dtv==1){
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


                if (ctx->tune_freq>0){
                    av_log(avctx, AV_LOG_INFO, "Set frequency %ld\n", ctx->tune_freq);
                    r = IDVBCLocator_put_CarrierFrequency(dvbc_locator, ctx->tune_freq );
                    if (r != S_OK) {
                        av_log(avctx, AV_LOG_ERROR, "Could not set DVB-C Locator\n");
                        goto error;
                    }
                }


                r = IDVBTuneRequest_put_Locator(dvb_tune_request, (ILocator *) dvbc_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set DVB-C Locator to DVBTuneRequest\n");
                    goto error;
                }

                r = IScanningTuner_Validate(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Error validating dvb tune request: r=0x%8x\n", r );
                    goto error;
                }

                r = IScanningTuner_put_TuneRequest(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set dvb tune request\n");
                    goto error;
                }
            }

            //// if tuningspace  == DVB-T
            if (ctx->dtv==2){
                ///IDVBTuneRequest_put_SID(dvb_tune_request, 316 );

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


                if (ctx->tune_freq>0){
                    av_log(avctx, AV_LOG_INFO, "Set frequency %ld\n", ctx->tune_freq);
                    r = IDVBTLocator_put_CarrierFrequency(dvbt_locator, ctx->tune_freq );
                    if (r != S_OK) {
                        av_log(avctx, AV_LOG_ERROR, "Could not set DVB-T Locator\n");
                        goto error;
                    }
                }


                r = IDVBTuneRequest_put_Locator(dvb_tune_request, (ILocator *) dvbt_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set DVB-T Locator to DVBTuneRequest\n");
                    goto error;
                }

                r = IScanningTuner_Validate(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Error validating dvb tune request: r=0x%8x\n", r );
                    goto error;
                }

                r = IScanningTuner_put_TuneRequest(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set dvb tune request\n");
                    goto error;
                }
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


                if (ctx->tune_freq>0){
                    av_log(avctx, AV_LOG_INFO, "Set frequency %ld\n", ctx->tune_freq);
                    r = IDVBSLocator_put_CarrierFrequency(dvbs_locator, ctx->tune_freq );
                    if (r != S_OK) {
                        av_log(avctx, AV_LOG_ERROR, "Could not set DVB-S Locator\n");
                        goto error;
                    }
                }


                r = IDVBTuneRequest_put_Locator(dvb_tune_request, (ILocator *) dvbs_locator);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set DVB-S Locator to DVBTuneRequest\n");
                    goto error;
                }

                r = IScanningTuner_Validate(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Error validating dvb tune request: r=0x%8x\n", r );
                    goto error;
                }

                r = IScanningTuner_put_TuneRequest(scanning_tuner, (ITuneRequest *) dvb_tune_request);
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set dvb tune request\n");
                    goto error;
                }
            }

        }

        //// if tuningspace == ATSC
        if (ctx->dtv == 4) {

            r = ITuneRequest_QueryInterface(tune_request, &IID_IATSCChannelTuneRequest, (void **) &atsc_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not query ATSCChannelTuneRequest.\n");
                goto error;
            }

            av_log(avctx, AV_LOG_INFO, "ATSCChannelTuneRequest acquired.\n");

            r = CoCreateInstance(&CLSID_ATSCLocator, NULL, CLSCTX_INPROC_SERVER,
                                 &IID_IATSCLocator, (void **) &atsc_locator);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not create ATSC Locator\n");
                goto error;
            }

            if (ctx->tune_freq>0){
                av_log(avctx, AV_LOG_INFO, "Set frequency %ld\n", ctx->tune_freq);
                r = IATSCLocator_put_CarrierFrequency(atsc_locator, ctx->tune_freq );
                if (r != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "Could not set ATSC Locator\n");
                    goto error;
                }
            }

            r = IATSCChannelTuneRequest_put_Locator(atsc_tune_request, (ILocator *) atsc_locator);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not set ATSC Locator to ATSCChannelTuneRequest\n");
                goto error;
            }

            r = IScanningTuner_Validate(scanning_tuner, (ITuneRequest *) atsc_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Error validating atsc tune request: r=0x%8x\n", r );
                goto error;
            }

            r = IScanningTuner_put_TuneRequest(scanning_tuner, (ITuneRequest *) atsc_tune_request);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Could not set atsc tune request\n");
                goto error;
            }
            av_log(avctx, AV_LOG_INFO, "success setting ATSC tune request");
        }


        ///////
        ////////////////////////////////////////////

        capture_filter = libAVFilter_Create(avctx, callback, VideoDevice);
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

        av_log(avctx, AV_LOG_INFO, "Video capture filter added to graph\n");


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

        if (ctx->dtv_graph_file) {
            const WCHAR wszStreamName[] = L"ActiveMovieGraph";
            IStorage *p_storage = NULL;
            IStream *ofile_stream = NULL;
            IPersistStream *pers_stream = NULL;
            WCHAR *gfilename = NULL;

            gfilename = malloc((strlen(ctx->dtv_graph_file)+4)*sizeof(WCHAR));

            mbstowcs(gfilename, ctx->dtv_graph_file, strlen(ctx->dtv_graph_file)+4);


            r = StgCreateDocfile(gfilename, STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                    0, &p_storage);
            if (S_OK != r) {
                av_log(avctx, AV_LOG_ERROR, "Could not create graph dump file.\n");
                goto error;
            }

            r = IStorage_CreateStream(p_storage, wszStreamName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &ofile_stream);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Creating IStream failed.\n");
                goto error;
            }

            r  = IBaseFilter_QueryInterface(graph, &IID_IPersistStream, (void **) &pers_stream);
            if (r != S_OK) {
                av_log(avctx, AV_LOG_ERROR, "Graph query for IPersistStream failed.\n");
                goto error;
            }

            if (!pers_stream) {
                av_log(avctx, AV_LOG_ERROR, "IPersistStream == NULL.\n");
                goto error;
            }


            //r = OleSaveToStream(pers_stream, ofile_stream);
            r = IPersistStream_Save(pers_stream, ofile_stream, TRUE);


            //if (SUCCEEDED(r)) {
                r = IStorage_Commit(p_storage, STGC_DEFAULT);       /// for some reason, it does not save if checked properly
            //}
//            if (r != S_OK) {
//                av_log(avctx, AV_LOG_ERROR, "Could not save capture dtv graph \n");
//                goto error;
//            }
//
//            r = IStorage_Commit(ofile_stream, STGC_DEFAULT);
//            if (S_OK != r) {
//                av_log(avctx, AV_LOG_ERROR, "Could not commit dtv graph data to file.\n");
//                goto error;
//            }

            IStream_Release(ofile_stream);
            IPersistStream_Release(pers_stream);

            IStorage_Release(p_storage);

            free(gfilename);

            av_log(avctx, AV_LOG_INFO, "Graph dump has been saved successfully.\n");

        }


        av_log(avctx, AV_LOG_INFO, "DTV Video capture filter connected\n");

        if ((r = dshow_add_device(avctx, VideoDevice)) < 0) {
            av_log(avctx, AV_LOG_ERROR, "Could not add video device.\n");
            goto error;
        }


        ////needs properties for demux too....???

    }

    else {
        ctx->video_codec_id = avctx->video_codec_id ? avctx->video_codec_id
                                                    : AV_CODEC_ID_RAWVIDEO;
        if (ctx->pixel_format != AV_PIX_FMT_NONE) {
            if (ctx->video_codec_id != AV_CODEC_ID_RAWVIDEO) {
                av_log(avctx, AV_LOG_ERROR, "Pixel format may only be set when "
                                  "video codec is not set or set to rawvideo\n");
                ret = AVERROR(EINVAL);
                goto error;
            }
        }
        if (ctx->framerate) {
            r = av_parse_video_rate(&ctx->requested_framerate, ctx->framerate);
            if (r < 0) {
                av_log(avctx, AV_LOG_ERROR, "Could not parse framerate '%s'.\n", ctx->framerate);
                goto error;
            }
        }

        r = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                             &IID_IGraphBuilder, (void **) &graph);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not create capture graph.\n");
            goto error;
        }
        ctx->graph = graph;

        r = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                             &IID_ICreateDevEnum, (void **) &devenum);
        if (r != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "Could not enumerate system devices.\n");
            goto error;
        }

        if (ctx->list_devices) {
            av_log(avctx, AV_LOG_INFO, "DirectShow video devices (some may be both video and audio devices)\n");
            dshow_cycle_devices(avctx, devenum, VideoDevice, VideoSourceDevice, NULL);
            av_log(avctx, AV_LOG_INFO, "DirectShow audio devices\n");
            dshow_cycle_devices(avctx, devenum, AudioDevice, AudioSourceDevice, NULL);
            ret = AVERROR_EXIT;
            goto error;
        }
        if (ctx->list_options) {
            if (ctx->device_name[VideoDevice])
                if ((r = dshow_list_device_options(avctx, devenum, VideoDevice, VideoSourceDevice))) {
                    ret = r;
                    goto error;
                }
            if (ctx->device_name[AudioDevice]) {
                if (dshow_list_device_options(avctx, devenum, AudioDevice, AudioSourceDevice)) {
                    /* show audio options from combined video+audio sources as fallback */
                    if ((r = dshow_list_device_options(avctx, devenum, AudioDevice, VideoSourceDevice))) {
                        ret = r;
                        goto error;
                    }
                }
            }
        }
        if (ctx->device_name[VideoDevice]) {
            if ((r = dshow_open_device(avctx, devenum, VideoDevice, VideoSourceDevice)) < 0 ||
                (r = dshow_add_device(avctx, VideoDevice)) < 0) {
                ret = r;
                goto error;
            }
        }
        if (ctx->device_name[AudioDevice]) {
            if ((r = dshow_open_device(avctx, devenum, AudioDevice, AudioSourceDevice)) < 0 ||
                (r = dshow_add_device(avctx, AudioDevice)) < 0) {
                av_log(avctx, AV_LOG_INFO, "Searching for audio device within video devices for %s\n", ctx->device_name[AudioDevice]);
                /* see if there's a video source with an audio pin with the given audio name */
                if ((r = dshow_open_device(avctx, devenum, AudioDevice, VideoSourceDevice)) < 0 ||
                    (r = dshow_add_device(avctx, AudioDevice)) < 0) {
                    ret = r;
                    goto error;
                }
            }
        }
        if (ctx->list_options) {
            /* allow it to list crossbar options in dshow_open_device */
            ret = AVERROR_EXIT;
            goto error;
        }

    }


    ctx->curbufsize[0] = 0;
    ctx->curbufsize[1] = 0;
    ctx->mutex = CreateMutex(NULL, 0, NULL);
    if (!ctx->mutex) {
        av_log(avctx, AV_LOG_ERROR, "Could not create Mutex\n");
        goto error;
    }
    ctx->event[1] = CreateEvent(NULL, 1, 0, NULL);
    if (!ctx->event[1]) {
        av_log(avctx, AV_LOG_ERROR, "Could not create Event\n");
        goto error;
    }

    r = IGraphBuilder_QueryInterface(graph, &IID_IMediaControl, (void **) &control);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not get media control.\n");
        goto error;
    }
    ctx->control = control;

    r = IGraphBuilder_QueryInterface(graph, &IID_IMediaEvent, (void **) &media_event);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not get media event.\n");
        goto error;
    }
    ctx->media_event = media_event;

    r = IMediaEvent_GetEventHandle(media_event, (void *) &media_event_handle);
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not get media event handle.\n");
        goto error;
    }
    proc = GetCurrentProcess();
    r = DuplicateHandle(proc, media_event_handle, proc, &ctx->event[0],
                        0, 0, DUPLICATE_SAME_ACCESS);
    if (!r) {
        av_log(avctx, AV_LOG_ERROR, "Could not duplicate media event handle.\n");
        goto error;
    }
    av_log(avctx, AV_LOG_INFO, "starting/running graph");
    r = IMediaControl_Run(control);
    if (r == S_FALSE) {
        OAFilterState pfs;
        r = IMediaControl_GetState(control, 0, &pfs);
    }
    if (r != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "Could not run graph (sometimes caused by a device already in use by other application)\n");
        goto error;
    }

    ret = 0;

error:

    if (devenum)
        ICreateDevEnum_Release(devenum);

    if (ret < 0)
        dshow_read_close(avctx);

    return ret;
}

/**
 * Checks media events from DirectShow and returns -1 on error or EOF. Also
 * purges all events that might be in the event queue to stop the trigger
 * of event notification.
 */
static int dshow_check_event_queue(IMediaEvent *media_event)
{
    LONG_PTR p1, p2;
    long code;
    int ret = 0;

    while (IMediaEvent_GetEvent(media_event, &code, &p1, &p2, 0) != E_ABORT) {
        if (code == EC_COMPLETE || code == EC_DEVICE_LOST || code == EC_ERRORABORT)
            ret = -1;
        IMediaEvent_FreeEventParams(media_event, code, p1, p2);
    }

    return ret;
}

static int dshow_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    struct dshow_ctx *ctx = s->priv_data;
    AVPacketList *pktl = NULL;

    while (!ctx->eof && !pktl) {
        WaitForSingleObject(ctx->mutex, INFINITE);
        pktl = ctx->pktl;
        if (pktl) {
            *pkt = pktl->pkt;
            ctx->pktl = ctx->pktl->next;
            av_free(pktl);
            ctx->curbufsize[pkt->stream_index] -= pkt->size;
        }
        ResetEvent(ctx->event[1]);
        ReleaseMutex(ctx->mutex);
        if (!pktl) {
            if (dshow_check_event_queue(ctx->media_event) < 0) {
                ctx->eof = 1;
            } else if (s->flags & AVFMT_FLAG_NONBLOCK) {
                return AVERROR(EAGAIN);
            } else {
                WaitForMultipleObjects(2, ctx->event, 0, INFINITE);
            }
        }
    }

    return ctx->eof ? AVERROR(EIO) : pkt->size;
}

static int dshow_url_open(URLContext *h, const char *filename, int flags)
{
    struct dshow_ctx *s = h->priv_data;
    if (!(s->protocol_av_format_context = avformat_alloc_context())) // TODO free
     return AVERROR(ENOMEM);
    av_strstart(filename, "dshowbda:", &filename); // remove prefix "dshowbda:"
    av_log(h, AV_LOG_INFO, "got parsed filename %s\n", filename); // works
    if (filename)
      av_strlcpy(&s->protocol_av_format_context->filename, filename, 1024);
    av_log(h, AV_LOG_INFO, "got parsed filename %s copied = %s\n", filename, s->protocol_av_format_context->filename);
    s->protocol_av_format_context->priv_data = s; // this is a bit circular, but needed to pass through the settings
    return dshow_read_header(s->protocol_av_format_context);
}

static int dshow_url_read(URLContext *h, uint8_t *buf, int size) 
{
    struct dshow_ctx *s = h->priv_data;
    int ret;
    av_log(h, AV_LOG_INFO, "dshow_url_read returning fail\n");
    return -1;
}

static int dshow_url_close(URLContext *h)
{
    struct dshow_ctx *s = h->priv_data;
    int ret;
    return -1;
}

#define OFFSET(x) offsetof(struct dshow_ctx, x)
#define DEC AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
    { "video_size", "set video size given a string such as 640x480 or hd720.", OFFSET(requested_width), AV_OPT_TYPE_IMAGE_SIZE, {.str = NULL}, 0, 0, DEC },
    { "pixel_format", "set video pixel format", OFFSET(pixel_format), AV_OPT_TYPE_PIXEL_FMT, {.i64 = AV_PIX_FMT_NONE}, -1, INT_MAX, DEC },
    { "framerate", "set video frame rate", OFFSET(framerate), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "sample_rate", "set audio sample rate", OFFSET(sample_rate), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "sample_size", "set audio sample size", OFFSET(sample_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 16, DEC },
    { "channels", "set number of audio channels, such as 1 or 2", OFFSET(channels), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "audio_buffer_size", "set audio device buffer latency size in milliseconds (default is the device's default)", OFFSET(audio_buffer_size), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "list_devices", "list available devices", OFFSET(list_devices), AV_OPT_TYPE_INT, {.i64=0}, 0, 1, DEC, "list_devices" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "list_devices" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "list_devices" },
    { "list_options", "list available options for specified device", OFFSET(list_options), AV_OPT_TYPE_INT, {.i64=0}, 0, 1, DEC, "list_options" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "list_options" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "list_options" },
    { "video_device_number", "set video device number for devices with same name (starts at 0)", OFFSET(video_device_number), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "audio_device_number", "set audio device number for devices with same name (starts at 0)", OFFSET(audio_device_number), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "video_pin_name", "select video capture pin by name", OFFSET(video_pin_name),AV_OPT_TYPE_STRING, {.str = NULL},  0, 0, AV_OPT_FLAG_ENCODING_PARAM },
    { "audio_pin_name", "select audio capture pin by name", OFFSET(audio_pin_name),AV_OPT_TYPE_STRING, {.str = NULL},  0, 0, AV_OPT_FLAG_ENCODING_PARAM },
    { "crossbar_video_input_pin_number", "set video input pin number for crossbar device", OFFSET(crossbar_video_input_pin_number), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, DEC },
    { "crossbar_audio_input_pin_number", "set audio input pin number for crossbar device", OFFSET(crossbar_audio_input_pin_number), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, DEC },
    { "show_video_device_dialog", "display property dialog for video capture device", OFFSET(show_video_device_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_video_device_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_video_device_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_video_device_dialog" },
    { "show_audio_device_dialog", "display property dialog for audio capture device", OFFSET(show_audio_device_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_audio_device_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_audio_device_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_audio_device_dialog" },
    { "show_video_crossbar_connection_dialog", "display property dialog for crossbar connecting pins filter on video device", OFFSET(show_video_crossbar_connection_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_video_crossbar_connection_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_video_crossbar_connection_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_video_crossbar_connection_dialog" },
    { "show_audio_crossbar_connection_dialog", "display property dialog for crossbar connecting pins filter on audio device", OFFSET(show_audio_crossbar_connection_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_audio_crossbar_connection_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_audio_crossbar_connection_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_audio_crossbar_connection_dialog" },
    { "show_analog_tv_tuner_dialog", "display property dialog for analog tuner filter", OFFSET(show_analog_tv_tuner_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_analog_tv_tuner_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_analog_tv_tuner_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_analog_tv_tuner_dialog" },
    { "show_analog_tv_tuner_audio_dialog", "display property dialog for analog tuner audio filter", OFFSET(show_analog_tv_tuner_audio_dialog), AV_OPT_TYPE_INT, {.i64 = 0}, 0, 1, DEC, "show_analog_tv_tuner_dialog" },
    { "true", "", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "show_analog_tv_tuner_audio_dialog" },
    { "false", "", 0, AV_OPT_TYPE_CONST, {.i64=0}, 0, 0, DEC, "show_analog_tv_tuner_audio_dialog" },
    { "audio_device_load", "load audio capture filter device (and properties) from file", OFFSET(audio_filter_load_file), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "audio_device_save", "save audio capture filter device (and properties) to file", OFFSET(audio_filter_save_file), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "video_device_load", "load video capture filter device (and properties) from file", OFFSET(video_filter_load_file), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "video_device_save", "save video capture filter device (and properties) to file", OFFSET(video_filter_save_file), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "dtv", "use digital tuner instead of analog", OFFSET(dtv), AV_OPT_TYPE_INT, {.i64 = 1}, 0, 4, DEC, "dtv" },
    { "c", "DVB-C", 0, AV_OPT_TYPE_CONST, {.i64=1}, 0, 0, DEC, "dtv" },
    { "t", "DVB-T", 0, AV_OPT_TYPE_CONST, {.i64=2}, 0, 0, DEC, "dtv" },
    { "s", "DVB-S", 0, AV_OPT_TYPE_CONST, {.i64=3}, 0, 0, DEC, "dtv" },
    { "a", "ATSC", 0, AV_OPT_TYPE_CONST, {.i64=4}, 0, 0, DEC, "dtv" },
    { "tune_freq", "set channel frequency (kHz)", OFFSET(tune_freq), AV_OPT_TYPE_INT, {.i64 = 0}, 0, INT_MAX, DEC },
    { "receiver_component", "BDA receive component filter name", OFFSET(receiver_component), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { "dump_dtv_graph", "save dtv graph to file", OFFSET(dtv_graph_file), AV_OPT_TYPE_STRING, {.str = NULL}, 0, 0, DEC },
    { NULL },
};

static const AVClass dshow_class = {
    .class_name = "dshow indev",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
    .category   = AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
};

AVInputFormat ff_dshow_demuxer = {
    .name           = "dshow",
    .long_name      = NULL_IF_CONFIG_SMALL("DirectShow capture"),
    .priv_data_size = sizeof(struct dshow_ctx),
    .read_header    = dshow_read_header,
    .read_packet    = dshow_read_packet,
    .read_close     = dshow_read_close,
    .flags          = AVFMT_NOFILE,
    .priv_class     = &dshow_class,
};



URLProtocol ff_dshow_protocol = {
    .name                = "dshowbda",
    .url_open            = dshow_url_open,
    .url_read            = dshow_url_read,
    .url_write           = NULL, // none yet
    .url_close           = dshow_url_close,
    .priv_data_size      = sizeof(struct dshow_ctx),
    .priv_data_class     = &dshow_class,
    .flags               = 0, // doesn't use network, no nested naming schema
};

