// for now enable like configure --extra-libs=-llzo2
#include "rzip.h"
#include "avcodec.h" // AV_CODEC_CAP_INTRA_ONLY
#include "libavutil/opt.h" // AVOption
#include "internal.h" // AV_CODEC_CAP_FRAME_THREADS
#include <lzo/lzo1x.h>

static const AVOption options[] = {
    { NULL },
};

static const AVClass rzipclass = {
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
    lzo_init();// XXXX threadsafe?
    return 0;
}

static int encode_frame_rzip(AVCodecContext *avctx, AVPacket *pkt,
                        const AVFrame *frame, int *got_packet)
{
    RzipContext *s = avctx->priv_data;
    int ret;
    lzo_uint clen = 0; // compressed length
    long tmp[LZO1X_1_MEM_COMPRESS]; // its temp working space, has to be this size
    int incoming_size = avpicture_get_size(frame->format, frame->width, frame->height);

    if (incoming_size < 0)
        return incoming_size;
    if ((ret = ff_alloc_packet2(avctx, pkt, incoming_size + incoming_size/16 + 64 + 3, 0)) < 0) // extra data in case compression inflates it
        return ret;

    av_log(avctx, AV_LOG_VERBOSE, "about to compress size %d\n", incoming_size);
    ret = lzo1x_1_compress(frame->data, incoming_size, pkt->data, &clen, tmp);
    if (ret != LZO_E_OK) {
      av_log(avctx, AV_LOG_INFO, "compression failed?");
      return -1;
    }
    pkt->flags |= AV_PKT_FLAG_KEY;
    pkt->size   = clen;
    av_log(avctx, AV_LOG_VERBOSE, "compressing to lzo was %d -> %d (compressed)\n", incoming_size, clen);

    return 0;
}

static av_cold int encode_end(AVCodecContext *avctx)
{
    RzipContext *s = avctx->priv_data;
    return 0;
}

AVCodec ff_rzip_encoder = {
    .name           = "rzip",
    .long_name      = NULL_IF_CONFIG_SMALL("RZIP [Roger Zip] encoder"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RZIP,
    .priv_data_size = sizeof(RzipContext),
    .init           = encode_init,
    .encode2        = encode_frame_rzip,
    .close          = encode_end,
    .capabilities   = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY,
    .priv_class     = &rzipclass,
    .pix_fmts       = (const enum AVPixelFormat[]){
        //AV_PIX_FMT_YUV422P, AV_PIX_FMT_RGB32, AV_PIX_FMT_NONE
        AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE // none just means "end of list"
    },
//    .caps_internal  = //FF_CODEC_CAP_INIT_THREADSAFE | // multiple init method calls OK
 //                     FF_CODEC_CAP_INIT_CLEANUP | // still call close even if open failed
  //                    AV_CODEC_CAP_DELAY, // send a NULL at the end meaning "close it up"
};

