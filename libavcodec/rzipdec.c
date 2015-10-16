
#include "rzip.h"
#include "avcodec.h"
#include "libavutil/lzo.h"
#include "libavutil/imgutils.h" // av_image_check_size
#include "internal.h" // ff_get_buffer 

static av_cold int decode_init(AVCodecContext *avctx)
{
    //RzipContext *s = avctx->priv_data;
    int ret;

    ret = av_image_check_size(avctx->width, avctx->height, 0, avctx);
    if (ret < 0)
        return ret;

    avctx->pix_fmt = AV_PIX_FMT_RGB24;

    // everybody seems to just use "extradata" [XXXX adjust the main docu for the same yikes]
    // guess its stable for height/width as well...
    // some use the first few bytes of each frame [hrm...]

    return 0;
}

static av_cold int decode_end(AVCodecContext *avctx)
{
    //RzipContext *s = avctx->priv_data;
    return 0;
}

static int rzip_decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    //RzipContext *s = avctx->priv_data;
    AVFrame *frame = data;
    const uint8_t *src = avpkt->data;
    int inlen = avpkt->size;
    int expected_output_size = avctx->width * avctx->height * 3; // RGB24
    int outlen = expected_output_size;
    int ret;

    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0) // allocate us a buffer which matches expected output size, apparently
        return ret;

    ret = av_lzo1x_decode(frame->data[0], &outlen, src, &inlen);
    if (ret < 0)
        return ret;
    // this outlen is actually invalid, somehow, but still works
    // av_log(avctx, AV_LOG_INFO, "decompressed from %d to %d (uncompressed) %d\n", avpkt->size, outlen, inlen); 
    av_log(avctx, AV_LOG_VERBOSE, "decompressed from %d to (presumably) %d\n", avpkt->size, expected_output_size);

    frame->pict_type        = AV_PICTURE_TYPE_I;
    frame->key_frame        = 1;

    *got_frame = 1;
    return outlen; // XXXX add some doc for "decode" [ret value. that is]
}

AVCodec ff_rzip_decoder = {
    .name             = "rzip",
    .long_name        = NULL_IF_CONFIG_SMALL("rzip [Roger zip] decoder"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_RZIP,
    .priv_data_size   = sizeof(RzipContext),
    .init             = decode_init,
    .close            = decode_end,
    .decode           = rzip_decode_frame,
    .capabilities     =  AV_CODEC_CAP_EXPERIMENTAL | AV_CODEC_CAP_LOSSLESS 
                 | AV_CODEC_CAP_DR1  // allow customer buffer allocation, seems benign
// TODO more? thread stuff?
//AV_CODEC_CAP_DR1 | AV_CODEC_CAP_DRAW_HORIZ_BAND | // TODO 
 //                       AV_CODEC_CAP_FRAME_THREADS,
//    .init_thread_copy = ONLY_IF_THREADS_ENABLED(decode_init_thread_copy),
};
