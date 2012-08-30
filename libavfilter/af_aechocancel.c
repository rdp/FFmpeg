/*
 * Copyright (c) 2011 Nicolas George <nicolas.george@normalesup.org>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Audio echo canceling filter
 */

#include "libavutil/audioconvert.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h" // only for SWR_CH_MAX
#include "avfilter.h"
#include "audio.h"
#include "bufferqueue.h"
#include "internal.h"
#include <speex/speex_echo.h>

typedef struct {
    const AVClass *class;
    int nb_inputs; // LODO remove
    int num_frames;
    int frame_size;
    int route[SWR_CH_MAX]; /**< channels routing, see copy_samples */  //LODO remove
    int bps;
    int largest_frame;
    SpeexEchoState *echo_state;
    struct aechocancel_input {
        struct FFBufQueue queue;
        int nb_ch;         /**< number of channels for the input */
        int nb_samples;
        int pos;
    } *in; // an array of them TODO [2]
} AEchoCancelContext;

#define OFFSET(x) offsetof(AEchoCancelContext, x)
#define FLAGS AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption aechocancel_options[] = {
    { "num_frames", "specify the number of samples to buffer", OFFSET(num_frames),
      AV_OPT_TYPE_INT, { .dbl = 3 }, 2, 1000000, FLAGS },
    { "frame_size", "specify the size all frames will be in -- use the asetnsamples filter if needed to force this", OFFSET(frame_size),
      AV_OPT_TYPE_INT, { .dbl = 1000 }, 2, 1000000, FLAGS },
    {0}
};

AVFILTER_DEFINE_CLASS(aechocancel);

static av_cold void uninit(AVFilterContext *ctx)
{
    AEchoCancelContext *am = ctx->priv;
    int i;
    speex_echo_state_destroy(am->echo_state);
    for (i = 0; i < am->nb_inputs; i++)
        ff_bufqueue_discard_all(&am->in[i].queue);
    av_freep(&am->in);
}

static int query_formats(AVFilterContext *ctx)
{
    AEchoCancelContext *am = ctx->priv;
    int64_t inlayout[SWR_CH_MAX], outlayout = 0;
    AVFilterFormats *formats;
    AVFilterChannelLayouts *layouts;
    int i, overlap = 0, nb_ch = 0;
    int sample_rates[] = { 44100, -1 };

    if(am->nb_inputs !=2 )
        av_log(ctx, AV_LOG_ERROR, "Requires 2 input channels\n");

    formats = ff_make_format_list((int[]){ AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE });
    ff_set_common_formats(ctx, formats);
    ff_set_common_samplerates(ctx, ff_make_format_list(sample_rates));//f_all_samplerates());
    return 0;
}

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    AEchoCancelContext *am = ctx->priv;
    int i;
    int nb_channels;

    for (i = 1; i < am->nb_inputs; i++) {
        if (ctx->inputs[i]->sample_rate != ctx->inputs[0]->sample_rate) {
            av_log(ctx, AV_LOG_ERROR,
                   "Inputs must have the same sample rate "
                   "%d for in%d vs %d\n",
                   ctx->inputs[i]->sample_rate, i, ctx->inputs[0]->sample_rate);
            return AVERROR(EINVAL);
        }

        nb_channels = av_get_channel_layout_nb_channels(ctx->inputs[i]->channel_layout);
        if (nb_channels != 1) {
          av_log(ctx, AV_LOG_ERROR, "Inputs must be mono, not stereo %d\n", nb_channels);
        }
    }
    am->bps = av_get_bytes_per_sample(ctx->outputs[0]->format);
    outlink->sample_rate = ctx->inputs[0]->sample_rate;
    outlink->time_base   = ctx->inputs[0]->time_base;
    am->echo_state = speex_echo_state_init(am->frame_size, am->num_frames); // bytes per sample yikes

    return 0;
}

static int request_frame(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    AEchoCancelContext *am = ctx->priv;
    int i, ret = 0;
    for (i = 1; i >= 0; i--) {// prefer the feeder guy, though it doesn't help <sigh>
       ret = ff_request_frame(ctx->inputs[i]);
       av_log(ctx, AV_LOG_ERROR, "requested from %d got %d\n", i, ret);
    }
    return ret;
}

static int filter_samples(AVFilterLink *inlink, AVFilterBufferRef *insamples)
{
    AVFilterContext *ctx = inlink->dst;
    AEchoCancelContext *am = ctx->priv;
    AVFilterLink *const outlink = ctx->outputs[0];
    int input_number;
    int nb_samples, ns, i;
    AVFilterBufferRef *outbuf, *inbuf[SWR_CH_MAX];
    uint8_t *ins[SWR_CH_MAX], *outs;

    // figure out which one is sending us the sample
    for (input_number = 0; input_number < am->nb_inputs; input_number++)
        if (inlink == ctx->inputs[input_number])
            break;
    av_assert0(input_number < 2);
    av_assert0(av_get_bytes_per_sample(insamples->format) == 2);
    if (insamples->audio->nb_samples != am->frame_size) {
      av_log(ctx, AV_LOG_ERROR, "requested to add %d samples which doesn't match number expected (%d)\n", insamples->audio->nb_samples, am->frame_size);
      return -1; 
    }
    // cancel all audio from [1] remove it from [0]
    if(input_number == 1) {
      // add it to the cancel queue
      av_log(ctx, AV_LOG_ERROR, "adding %d samples to cancel q\n", insamples->audio->nb_samples);
      speex_echo_playback(am->echo_state, insamples->data[0]);
      return 0; // ok
    } else {
      av_log(ctx, AV_LOG_ERROR, "parsing %d samples to remove it from\n", insamples->audio->nb_samples);
      speex_echo_capture(am->echo_state, insamples->data[0], insamples->data[0] + i*2);
      return ff_filter_samples(ctx->outputs[0], insamples); // Send a buffer of audio samples to the next filter. 
    }
}

static av_cold int init(AVFilterContext *ctx, const char *args)
{
    AEchoCancelContext *am = ctx->priv;
    int ret, i;
    char name[16];

    am->class = &aechocancel_class;
    am->nb_inputs = 2; // LODO remove
    av_opt_set_defaults(am);
    ret = av_set_options_string(am, args, "=", ":");
    if (ret < 0) {
        av_log(ctx, AV_LOG_ERROR, "Error parsing options: '%s'\n", args);
        return ret;
    }
    am->in = av_calloc(am->nb_inputs, sizeof(*am->in));
    if (!am->in)
        return AVERROR(ENOMEM);
    av_log(ctx, AV_LOG_ERROR, "setting names %d\n", am->nb_inputs);
    for (i = 0; i < am->nb_inputs; i++) {
    av_log(ctx, AV_LOG_ERROR, "setting name %d\n", i);
        snprintf(name, sizeof(name), "in%d", i);
        AVFilterPad pad = {
            .name             = av_strdup(name),
            .type             = AVMEDIA_TYPE_AUDIO,
            .filter_samples   = filter_samples,
            .min_perms        = AV_PERM_READ | AV_PERM_PRESERVE | AV_PERM_WRITE,
        };
        ff_insert_inpad(ctx, i, &pad);
    av_log(ctx, AV_LOG_ERROR, "setting name2 %d\n", i);
    }
    return 0;
}

AVFilter avfilter_af_aechocancel = {
    .name          = "aechocancel",
    .description   = NULL_IF_CONFIG_SMALL("Cancels one audio stream out of "
                                          "another."),
    .priv_size     = sizeof(AEchoCancelContext),
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,

    .inputs    = (const AVFilterPad[]) { { .name = NULL } },
    .outputs   = (const AVFilterPad[]) {
        { .name             = "default",
          .type             = AVMEDIA_TYPE_AUDIO,
          .config_props     = config_output,
          .request_frame    = request_frame
         },
        { .name = NULL }
    },
    .priv_class = &aechocancel_class,
};
