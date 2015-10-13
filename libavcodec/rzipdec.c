
#include "rzip.h"
#include "avcodec.h"

static av_cold int decode_init(AVCodecContext *avctx)
{
    RzipContext *s = avctx->priv_data;
    avctx->pix_fmt = AV_PIX_FMT_RGB24; // what does this mean, internal pix fmt?
    return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
    RzipContext *s = avctx->priv_data;
    return 0;
}

static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    RzipContext *s = avctx->priv_data;
    return 0;
}

AVCodec ff_rzip_decoder = {
    .name             = "rzip",
    .long_name        = NULL_IF_CONFIG_SMALL("rzip"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_RZIP,
    .priv_data_size   = sizeof(RzipContext),
    .init             = decode_init,
    .close            = decode_end,
    .decode           = decode_frame,
    .capabilities     =  AV_CODEC_CAP_EXPERIMENTAL | AV_CODEC_CAP_LOSSLESS  // TODO more, like pass it nulls for the last GOP batch [?]
//AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DRAW_HORIZ_BAND | // TODO 
 //                       AV_CODEC_CAP_FRAME_THREADS,
//    .init_thread_copy = ONLY_IF_THREADS_ENABLED(decode_init_thread_copy), // ??
};
