// for now enable like configure --extra-libs=-llzo2 TODO
#include "rzip.h"
#include "avcodec.h" // AV_CODEC_CAP_INTRA_ONLY
#include "libavutil/opt.h" // AVOption
#include "internal.h" // AV_CODEC_CAP_FRAME_THREADS
#include <lzo/lzo1x.h>
#include "libavutil/lzo.h"

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
    //RzipContext *s = avctx->priv_data;
    // no gop stuff yet, we're all gop
    //s->rzip_gop = 30*10; // 10s default, assuming x264 has good values :)
    //if (avctx->gop_size > 0)
    //  s->rzip_gop = avctx->gop_size;
    lzo_init(); // XXXX threadsafe? avoid multiples?
    av_log(avctx, AV_LOG_VERBOSE, "doing init\n");
    return 0;
}

static int rdp_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                        const AVFrame *frame, int *got_packet)
{
    //RzipContext *s = avctx->priv_data;
    int ret;
    lzo_uint clen = 0; // compressed length
    long tmp[LZO1X_1_MEM_COMPRESS]; // lzo temp working space, has to be this size
    int incoming_size = avpicture_get_size(frame->format, frame->width, frame->height);

    if (incoming_size < 0)
        return incoming_size;
    if ((ret = ff_alloc_packet2(avctx, pkt, incoming_size + incoming_size/16 + 64 + 3, 0)) < 0) // extra data in case compression inflates it
        return ret;
   
    ret = lzo1x_1_compress(frame->data[0], incoming_size, pkt->data, &clen, tmp);
    if (ret != LZO_E_OK) {
      av_log(avctx, AV_LOG_INFO, "compression failed?");
      return -1;
    }
    pkt->flags |= AV_PKT_FLAG_KEY;
    pkt->size   = clen;
    av_log(avctx, AV_LOG_VERBOSE, "compressing to lzo was %d -> %lu (compressed)\n", incoming_size, clen);


    int outlen = incoming_size;
    int inlen = clen;

    //ret = av_lzo1x_decode(pkt->data, &outlen, frame->data[0], &inlen); // todo make sure right size out [out final decomp, out, in, in]
    //if (ret < 0) 
    //  return ret;
    //av_log(avctx, AV_LOG_VERBOSE, "decompressing it went to %d\n", outlen);

    //outlen = incoming_size;
    //inlen = clen;
    //ret = lzo1x_decompress(frame->data[0], clen, pkt->data, &outlen, NULL); // from, from, ot, to
    //if (ret < 0) 
    //  return ret;
    //av_log(avctx, AV_LOG_VERBOSE, "decompressing it went to %d with theirs\n", outlen);

  *got_packet = 1; // I gave you a packet

    return 0;
}

static av_cold int encode_end(AVCodecContext *avctx)
{
    //RzipContext *s = avctx->priv_data;
    return 0;
}

AVCodec ff_rzip_encoder = {
    .name           = "rzip",
    .long_name      = NULL_IF_CONFIG_SMALL("RZIP [Roger Zip] encoder"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_RZIP,
    .priv_data_size = sizeof(RzipContext),
    .init           = encode_init,
    .encode2        = rdp_encode_frame,
    .close          = encode_end,
     // AV_CODEC_CAP_FRAME_THREADS I think means "interleae threads" or something
     // INTRA_ONLY I think means "no inter anything" between frames [?]
    .capabilities   = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY | AV_CODEC_CAP_EXPERIMENTAL,
    .priv_class     = &rzipclass,
    .pix_fmts       = (const enum AVPixelFormat[]){
        //AV_PIX_FMT_YUV422P, AV_PIX_FMT_RGB32, AV_PIX_FMT_NONE
        AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE // none just means "end of list"
    },
//    .caps_internal  = //FF_CODEC_CAP_INIT_THREADSAFE | // multiple init method calls OK
 //                     FF_CODEC_CAP_INIT_CLEANUP | // still call close even if open failed
  //                    AV_CODEC_CAP_DELAY, // send a NULL at the end meaning "close it up"
};

