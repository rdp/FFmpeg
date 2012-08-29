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
 * Audio merging filter
 */

#include "libavutil/audioconvert.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h" // only for SWR_CH_MAX
#include "avfilter.h"
#include "audio.h"
#include "bufferqueue.h"
#include "internal.h"

typedef struct {
    const AVClass *class;
    int nb_inputs;
    int route[SWR_CH_MAX]; /**< channels routing, see copy_samples */
    int bps;
    struct aechocancel_input {
        struct FFBufQueue queue;
        int nb_ch;         /**< number of channels for the input */
        int nb_samples;
        int pos;
    } *in;
} AEchoCancelContext;

#define OFFSET(x) offsetof(AEchoCancelContext, x)
#define FLAGS AV_OPT_FLAG_AUDIO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption aechocancel_options[] = {
    { "inputs", "specify the number of inputs", OFFSET(nb_inputs),
      AV_OPT_TYPE_INT, { .dbl = 2 }, 2, SWR_CH_MAX, FLAGS },
    {0}
};

AVFILTER_DEFINE_CLASS(aechocancel);

static av_cold void uninit(AVFilterContext *ctx)
{
    AEchoCancelContext *am = ctx->priv;
    int i;

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

    if(am->nb_inputs !=2 )
        av_log(ctx, AV_LOG_ERROR, "Requires 2 input channels\n");

    formats = ff_make_format_list((int[]){ AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE });
    ff_set_common_formats(ctx, formats);
    int sample_rates[] = { 44100, -1 };
    ff_set_common_samplerates(ctx, ff_make_format_list(sample_rates));//f_all_samplerates());
    return 0;
}

static int config_output(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    AEchoCancelContext *am = ctx->priv;
    int i;

    for (i = 1; i < am->nb_inputs; i++) {
        if (ctx->inputs[i]->sample_rate != ctx->inputs[0]->sample_rate) {
            av_log(ctx, AV_LOG_ERROR,
                   "Inputs must have the same sample rate "
                   "%d for in%d vs %d\n",
                   ctx->inputs[i]->sample_rate, i, ctx->inputs[0]->sample_rate);
            return AVERROR(EINVAL);
        }
    }
    am->bps = av_get_bytes_per_sample(ctx->outputs[0]->format);
    outlink->sample_rate = ctx->inputs[0]->sample_rate;
    outlink->time_base   = ctx->inputs[0]->time_base;

    av_log(ctx, AV_LOG_VERBOSE, "done init aecho\n");

    return 0;
}

static int request_frame(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    AEchoCancelContext *am = ctx->priv;
    int i, ret;

    for (i = 0; i < am->nb_inputs; i++)
        if (!am->in[i].nb_samples)
            if ((ret = ff_request_frame(ctx->inputs[i])) < 0)
                return ret;
    return 0;
}

/**
 * Copy samples from several input streams to one output stream.
 * @param nb_inputs number of inputs
 * @param in        inputs; used only for the nb_ch field;
 * @param route     routing values;
 *                  input channel i goes to output channel route[i];
 *                  i <  in[0].nb_ch are the channels from the first output;
 *                  i >= in[0].nb_ch are the channels from the second output
 * @param ins       pointer to the samples of each inputs, in packed format;
 *                  will be left at the end of the copied samples
 * @param outs      pointer to the samples of the output, in packet format;
 *                  must point to a buffer big enough;
 *                  will be left at the end of the copied samples
 * @param ns        number of samples to copy
 * @param bps       bytes per sample
 */
static inline void copy_samples(int nb_inputs, struct aechocancel_input in[],
                                int *route, uint8_t *ins[],
                                uint8_t **outs, int ns, int bps)
{
    int *route_cur;
    int i, c, nb_ch = 0;

    for (i = 0; i < nb_inputs; i++)
        nb_ch += in[i].nb_ch;
    while (ns--) {
        route_cur = route;
        for (i = 0; i < nb_inputs; i++) {
            for (c = 0; c < in[i].nb_ch; c++) {
                memcpy((*outs) + bps * *(route_cur++), ins[i], bps);
                ins[i] += bps;
            }
        }
        *outs += nb_ch * bps;
    }
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

    for (input_number = 0; input_number < am->nb_inputs; input_number++)
        if (inlink == ctx->inputs[input_number])
            break;
    av_assert1(input_number < am->nb_inputs);
    ff_bufqueue_add(ctx, &am->in[input_number].queue, insamples);
    am->in[input_number].nb_samples += insamples->audio->nb_samples;
    nb_samples = am->in[0].nb_samples;
    for (i = 1; i < am->nb_inputs; i++)
        nb_samples = FFMIN(nb_samples, am->in[i].nb_samples);
    if (!nb_samples)
        return 0;

    outbuf = ff_get_audio_buffer(ctx->outputs[0], AV_PERM_WRITE, nb_samples);
    outs = outbuf->data[0];
    for (i = 0; i < am->nb_inputs; i++) {
        inbuf[i] = ff_bufqueue_peek(&am->in[i].queue, 0);
        ins[i] = inbuf[i]->data[0] +
                 am->in[i].pos * am->in[i].nb_ch * am->bps;
    }
    avfilter_copy_buffer_ref_props(outbuf, inbuf[0]);
    outbuf->pts = inbuf[0]->pts == AV_NOPTS_VALUE ? AV_NOPTS_VALUE :
                  inbuf[0]->pts +
                  av_rescale_q(am->in[0].pos,
                               (AVRational){ 1, ctx->inputs[0]->sample_rate },
                               ctx->outputs[0]->time_base);

    outbuf->audio->nb_samples     = nb_samples;
    outbuf->audio->channel_layout = outlink->channel_layout;

    while (nb_samples) {
        ns = nb_samples;
        for (i = 0; i < am->nb_inputs; i++)
            ns = FFMIN(ns, inbuf[i]->audio->nb_samples - am->in[i].pos);
        /* Unroll the most common sample formats: speed +~350% for the loop,
           +~13% overall (including two common decoders) */
        switch (am->bps) {
            case 1:
                copy_samples(am->nb_inputs, am->in, am->route, ins, &outs, ns, 1);
                break;
            case 2:
                copy_samples(am->nb_inputs, am->in, am->route, ins, &outs, ns, 2);
                break;
            case 4:
                copy_samples(am->nb_inputs, am->in, am->route, ins, &outs, ns, 4);
                break;
            default:
                copy_samples(am->nb_inputs, am->in, am->route, ins, &outs, ns, am->bps);
                break;
        }

        nb_samples -= ns;
        for (i = 0; i < am->nb_inputs; i++) {
            am->in[i].nb_samples -= ns;
            am->in[i].pos += ns;
            if (am->in[i].pos == inbuf[i]->audio->nb_samples) {
                am->in[i].pos = 0;
                avfilter_unref_buffer(inbuf[i]);
                ff_bufqueue_get(&am->in[i].queue);
                inbuf[i] = ff_bufqueue_peek(&am->in[i].queue, 0);
                ins[i] = inbuf[i] ? inbuf[i]->data[0] : NULL;
            }
        }
    }
    return ff_filter_samples(ctx->outputs[0], outbuf);
}

static av_cold int init(AVFilterContext *ctx, const char *args)
{
    AEchoCancelContext *am = ctx->priv;
    int ret, i;
    char name[16];

    am->class = &aechocancel_class;
    av_opt_set_defaults(am);
    ret = av_set_options_string(am, args, "=", ":");
    if (ret < 0) {
        av_log(ctx, AV_LOG_ERROR, "Error parsing options: '%s'\n", args);
        return ret;
    }
    am->in = av_calloc(am->nb_inputs, sizeof(*am->in));
    if (!am->in)
        return AVERROR(ENOMEM);
    for (i = 0; i < am->nb_inputs; i++) {
        AVFilterPad pad = {
            .name             = name,
            .type             = AVMEDIA_TYPE_AUDIO,
            .filter_samples   = filter_samples,
            .min_perms        = AV_PERM_READ | AV_PERM_PRESERVE,
        };
        snprintf(name, sizeof(name), "in%d", i);
        ff_insert_inpad(ctx, i, &pad);
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
          .request_frame    = request_frame, },
        { .name = NULL }
    },
    .priv_class = &aechocancel_class,
};
