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

static const AVClass dshow_class = {
    .class_name = "dshow indev",
    .item_name  = av_default_item_name,
    .option     = dshow_options,
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

