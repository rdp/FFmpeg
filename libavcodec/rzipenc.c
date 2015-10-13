#include "rzip.h"
#include "avcodec.h" // AV_CODEC_CAP_INTRA_ONLY
#include "libavutil/opt.h" // AVOption
#include "internal.h" // AV_CODEC_CAP_FRAME_THREADS


static const AVOption options[] = {
    { NULL },
};

static const AVClass class = {
    .class_name = "rzip",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

static av_cold int encode_init(AVCodecContext *avctx)
{
    RzipContext *s = avctx->priv_data;
    s->rzip_gop = 30*10; // 10s default, assuming x264 has good values :)
    if (avctx->gop_size > 0)
      s->rzip_gop = avctx->gop_size;
    return 0;
}

static int encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                        const AVFrame *pict, int *got_packet)
{
    RzipContext *s = avctx->priv_data;
    // ...
    return 0;
}

static av_cold int encode_end(AVCodecContext *avctx)
{
    RzipContext *s = avctx->priv_data;
    return 0;
}

AVCodec ff_rzip_encoder = {
    .name           = "rzip",
    .long_name      = NULL_IF_CONFIG_SMALL("RZIP [Roger Zip] Lossless"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RZIP,
    .priv_data_size = sizeof(RzipContext),
    .init           = encode_init,
    .encode2        = encode_frame,
    .close          = encode_end,
    .capabilities   = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY,
    .priv_class     = &class,
    .pix_fmts       = (const enum AVPixelFormat[]){
        //AV_PIX_FMT_YUV422P, AV_PIX_FMT_RGB32, AV_PIX_FMT_NONE
        AV_PIX_FMT_RGB24
    },
    .caps_internal  = FF_CODEC_CAP_INIT_THREADSAFE | // multiple init method calls OK
                      FF_CODEC_CAP_INIT_CLEANUP, // still call close even if open failed
                      AV_CODEC_CAP_DELAY, // send a NULL at the end meaning "close it up"
};

