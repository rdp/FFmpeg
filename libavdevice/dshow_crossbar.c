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
    default:                              return "Unknown Crossbar Pin Type—Please report!";
    }
}

static HRESULT
setup_crossbar_options(IAMCrossbar *pXBar, int video_input_pin, int audio_input_pin, const char *device_name,
    int list_options, AVFormatContext *avctx)
{
    HRESULT hr;
    long count_output, count_input;
    int i;
    int log_level = list_options ? AV_LOG_INFO : AV_LOG_DEBUG;

    av_log(avctx, log_level, "Crossbar Switching Information for %s:\n", device_name); /* TODO only log if show_options set */
    hr = IAMCrossbar_get_PinCounts(pXBar, &count_output, &count_input);
    if (hr != S_OK) {
        av_log(avctx, AV_LOG_ERROR, "unable to get crossbar pin counts\n");
        return hr;
    }

    for (i = 0; i < count_output; i++)
    {
        long related_pin = -1, pin_type = -1, route_to_pin = -1;
        hr = IAMCrossbar_get_CrossbarPinInfo(pXBar, FALSE, i, &related_pin, &pin_type);
        if (pin_type == PhysConn_Video_VideoDecoder) {
            /* assume there is only one "Video (and one Audio) Decoder" output pin, and it's all we care about routing to...for now */
            if (video_input_pin != -1) {
                av_log(avctx, log_level, "routing video input from pin %d\n", video_input_pin);
                hr = IAMCrossbar_Route(pXBar, i, video_input_pin);
                if (hr != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "unable to route video input from pin %d\n", video_input_pin);
                    return AVERROR(EIO);
                }
            }
        } else if (pin_type == PhysConn_Audio_AudioDecoder) {
            if (audio_input_pin != -1) {
                av_log(avctx, log_level, "routing audio input from pin %d\n", audio_input_pin);
                hr = IAMCrossbar_Route(pXBar, i, audio_input_pin);
                if (hr != S_OK) {
                    av_log(avctx, AV_LOG_ERROR, "unable to route audio input from pin %d\n", audio_input_pin);
                    return hr;
                }
            }
        } else {
            av_log(avctx, AV_LOG_WARNING, "unexpected output pin type, please report the type if you want to use this (%s)", GetPhysicalPinName(pin_type));
        }

        hr = IAMCrossbar_get_IsRoutedTo(pXBar, i, &route_to_pin);
        if (hr != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "unable to get crossbar is routed to from pin %d\n", i);
            return hr;
        }
        av_log(avctx, log_level, "  Crossbar Output pin %d: \"%s\" related output pin: %ld ", i, GetPhysicalPinName(pin_type), related_pin);
        av_log(avctx, log_level, "current input pin: %ld ", route_to_pin);
        av_log(avctx, log_level, "compatible input pins: ");

        for (int j = 0; j < count_input; j++)
        {
            hr = IAMCrossbar_CanRoute(pXBar, i, j);
            if (hr == S_OK)
                av_log(avctx, log_level ,"%d ", j);
        }
        av_log(avctx, log_level, "\n");
    }

    for (i = 0; i < count_input; i++)
    {
        long related_pin, pin_type;
        hr = IAMCrossbar_get_CrossbarPinInfo(pXBar, TRUE, i, &related_pin, &pin_type);
        if (hr != S_OK) {
            av_log(avctx, AV_LOG_ERROR, "unable to get crossbar info audio input from pin %d\n", i);
            return hr;
        }
        av_log(avctx, log_level, "  Crossbar Input pin %d - \"%s\" ", i, GetPhysicalPinName(pin_type));
        av_log(avctx, log_level, "related input pin: %ld\n", related_pin);
    }
    return S_OK;
}

/**
 * Given a fully constructed graph, check if there is a cross bar filter, and configure its pins if so.
 */
HRESULT
dshow_try_setup_crossbar_options(ICaptureGraphBuilder2 *graph_builder2, IBaseFilter *device_filter,
    int crossbar_video_input_pin_number, int crossbar_audio_input_pin_number, const char *device_name,
    int list_options, int show_crossbar_connection_properties, AVFormatContext *avctx) {
    IAMCrossbar *cross_bar = NULL;
    IBaseFilter *cross_bar_filter = NULL;
    HRESULT hr;

    hr = ICaptureGraphBuilder2_FindInterface(graph_builder2, &LOOK_UPSTREAM_ONLY, (const GUID *) NULL,
            (IBaseFilter *) device_filter, &IID_IAMCrossbar, (void**) &cross_bar);
    if (hr != S_OK) {
        /* no crossbar found */
        hr = S_OK;
        goto end;
    }

    if (show_crossbar_connection_properties) {
        hr = IAMCrossbar_QueryInterface(cross_bar, &IID_IBaseFilter, (void **) &cross_bar_filter);
        if (hr != S_OK)
            goto end;
        dshow_show_filter_properties(cross_bar_filter, avctx);
    }
    hr = setup_crossbar_options(cross_bar, crossbar_video_input_pin_number,
        crossbar_audio_input_pin_number, device_name, list_options, avctx);
    if (hr != S_OK)
        goto end;

end:
    if (cross_bar)
        IAMCrossbar_Release(cross_bar);
    if (cross_bar_filter)
        IBaseFilter_Release(cross_bar_filter);
    return hr;
}
