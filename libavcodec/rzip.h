// TODO headers

#ifndef AVCODEC_RZIP_H
#define AVCODEC_RZIP_H

#include "avcodec.h" // AVClass

typedef struct RzipContext {
  AVClass *class;
  //int rzip_gop;
  int image_size; // decode only
} RzipContext;

#endif
