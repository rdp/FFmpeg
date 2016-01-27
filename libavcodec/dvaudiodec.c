/*
 * Copyright (c) 2012 Laurent Aimar
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

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "internal.h"

typedef struct DVAudioContext {
    int block_size;
    int is_12bit;
    int is_pal;
    int16_t shuffle[2000];
} DVAudioContext;

static av_cold int decode_init(AVCodecContext *avctx)
{
    DVAudioContext *s = avctx->priv_data;
    int i;

    if (avctx->channels != 2) {
        av_log(avctx, AV_LOG_ERROR, "invalid number of channels\n");
        return AVERROR(EINVAL);
    }

    if (avctx->codec_tag == 0x0215) {
        s->block_size = 7200;
        s->is_pal = 0;
    } else if (avctx->codec_tag == 0x0216) {
        s->block_size = 8640;
        s->is_pal = 1;
    } else {
        return AVERROR(EINVAL);
    }

    s->is_12bit = avctx->bits_per_raw_sample == 12;
    avctx->sample_fmt = AV_SAMPLE_FMT_S16;

    for (i = 0; i < FF_ARRAY_ELEMS(s->shuffle); i++) {
        const unsigned a = s->is_pal ? 18 : 15;
        const unsigned b = 3 * a;

        s->shuffle[i] = 80 * ((21 * (i % 3) + 9 * (i / 3) + ((i / a) % 3)) % b) +
                         (2 + s->is_12bit) * (i / b) + 8;
    }

    return 0;
}

static inline int dv_get_audio_sample_count(const uint8_t *buffer, int dsf)
{
    int samples = buffer[0] & 0x3f; /* samples in this frame - min samples */

    switch ((buffer[3] >> 3) & 0x07) {
    case 0:
        return samples + (dsf ? 1896 : 1580);
    case 1:
        return samples + (dsf ? 1742 : 1452);
    case 2:
    default:
        return samples + (dsf ? 1264 : 1053);
    }
}

static inline uint16_t dv_audio_12to16(uint16_t sample)
{
    uint16_t shift, result;

    sample = (sample < 0x800) ? sample : sample | 0xf000;
    shift  = (sample & 0xf00) >> 8;

    if (shift < 0x2 || shift > 0xd) {
        result = sample;
    } else if (shift < 0x8) {
        shift--;
        result = (sample - (256 * shift)) << shift;
    } else {
        shift  = 0xe - shift;
        result = ((sample + ((256 * shift) + 1)) << shift) - 1;
    }

    return result;
}

static int decode_frame(AVCodecContext *avctx, void *data,
                        int *got_frame_ptr, AVPacket *pkt)
{
    DVAudioContext *s = avctx->priv_data;
    AVFrame *frame = data;
    const uint8_t *src = pkt->data;
    int16_t *dst;
    int ret, i;

    if (pkt->size != s->block_size)
        return AVERROR_INVALIDDATA;

    frame->nb_samples = dv_get_audio_sample_count(pkt->data + 244, s->is_pal);
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;
    dst = (int16_t *)frame->data[0];

    for (i = 0; i < frame->nb_samples; i++) {
       const uint8_t *v = &src[s->shuffle[i]];

       if (s->is_12bit) {
           *dst++ = dv_audio_12to16((v[0] << 4) | ((v[2] >> 4) & 0x0f));
           *dst++ = dv_audio_12to16((v[1] << 4) | ((v[2] >> 0) & 0x0f));
       } else {
           *dst++ = AV_RB16(&v[0]);
           *dst++ = AV_RB16(&v[s->is_pal ? 4320 : 3600]);
       }
    }

    *got_frame_ptr = 1;

    return pkt->size;
}

AVCodec ff_dvaudio_decoder = {
    .name           = "dvaudio",
    .long_name      = NULL_IF_CONFIG_SMALL("Ulead DV Audio"),
    .type           = AVMEDIA_TYPE_AUDIO,
    .id             = AV_CODEC_ID_DVAUDIO,
    .init           = decode_init,
    .decode         = decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
    .priv_data_size = sizeof(DVAudioContext),
};
