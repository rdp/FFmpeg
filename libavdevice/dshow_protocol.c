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
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/avassert.h"

static int dshow_url_open(URLContext *h, const char *filename, int flags)
{
    struct dshow_ctx *ctx = h->priv_data;
    int r;
    
    if (!(ctx->protocol_av_format_context = avformat_alloc_context()))
     return AVERROR(ENOMEM);
    ctx->protocol_av_format_context->flags = h->flags; 

    av_strstart(filename, "dshowbda:", &filename); // remove prefix "dshowbda:"
    if (filename)
      av_strlcpy(ctx->protocol_av_format_context->filename, filename, 1024); // 1024 max bytes
    ctx->protocol_av_format_context->iformat = &ff_dshow_demuxer;
    ctx->protocol_latest_packet = av_packet_alloc();
    ctx->protocol_latest_packet->pos = 0; // default is -1
    if (!ctx->protocol_latest_packet)
      return AVERROR(ENOMEM);
    ctx->protocol_av_format_context->priv_data = ctx; // a bit circular, but needed to pass through the settings
    r = dshow_read_header(ctx->protocol_av_format_context);
    if (r == S_OK)
        dshow_log_signal_strength(ctx->protocol_av_format_context, AV_LOG_INFO);
    return r;
}

static int dshow_url_read(URLContext *h, uint8_t *buf, int max_size) 
{
    struct dshow_ctx *ctx = h->priv_data;
    int packet_size_or_fail;
    int bytes_to_copy;
    int bytes_left = ctx->protocol_latest_packet->size - ctx->protocol_latest_packet->pos;

    if (bytes_left == 0) {
      av_packet_unref(ctx->protocol_latest_packet);
      packet_size_or_fail = dshow_read_packet(ctx->protocol_av_format_context, ctx->protocol_latest_packet);
      if (packet_size_or_fail < 0) 
        return packet_size_or_fail;
      av_assert0(ctx->protocol_latest_packet->stream_index == 0); // this should be a stream based, so only one stream, so always index 0
      av_assert0(packet_size_or_fail == ctx->protocol_latest_packet->size); // should match...
      ctx->protocol_latest_packet->pos = 0; // default is -1
      bytes_left = ctx->protocol_latest_packet->size - ctx->protocol_latest_packet->pos;
      av_log(h, AV_LOG_DEBUG, "dshow_url_read read packet of size %d\n", ctx->protocol_latest_packet->size);
    }
    bytes_to_copy = FFMIN(bytes_left, max_size);
    if (bytes_to_copy != bytes_left)
        av_log(h, AV_LOG_DEBUG, "passing partial dshow packet %d > %d\n", bytes_left, max_size);
    memcpy(buf, &ctx->protocol_latest_packet->data[ctx->protocol_latest_packet->pos], bytes_to_copy);
    ctx->protocol_latest_packet->pos += bytes_to_copy; 
    if (ctx->video_frame_num % (30*60) == 0) { // once/min
      dshow_log_signal_strength(ctx->protocol_av_format_context, AV_LOG_VERBOSE);
    }
    return bytes_to_copy;;
}

static int dshow_url_close(URLContext *h)
{
    struct dshow_ctx *ctx = h->priv_data;
    int ret = dshow_read_close(ctx->protocol_av_format_context);
    ctx->protocol_av_format_context->priv_data = NULL; // just in case it would be freed below
    avformat_free_context(ctx->protocol_av_format_context);
    av_packet_free(&ctx->protocol_latest_packet); // also does an unref
    return ret;
}

void go_temp(void) {} // for linking purposes for now

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

