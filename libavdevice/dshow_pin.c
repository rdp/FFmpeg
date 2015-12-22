/*
 * DirectShow capture interface
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

#include <stddef.h>
#define imemoffset offsetof(libAVPin, imemvtbl)

DECLARE_QUERYINTERFACE(libAVPin,
    { {&IID_IUnknown,0}, {&IID_IPin,0}, {&IID_IMemInputPin,imemoffset} })
DECLARE_ADDREF(libAVPin)
DECLARE_RELEASE(libAVPin)

long WINAPI
libAVPin_Connect(libAVPin *this, IPin *pin, const AM_MEDIA_TYPE *type)
{
    dshowdebug("libAVPin_Connect returning false(%p, %p, %p)\n", this, pin, type);
    /* Input pins receive connections. */
    return S_FALSE;
}
long WINAPI
libAVPin_ReceiveConnection(libAVPin *this, IPin *pin,
                           const AM_MEDIA_TYPE *type)
{
    enum dshowDeviceType devtype = this->filter->type;
    dshowdebug("libAVPin_ReceiveConnection(%p) connectedto(%p)\n", this, this->connectedto);
    ff_print_AM_MEDIA_TYPE(type);

    if (!pin){
        dshowdebug(" no pin (%p)\n", this);
        return E_POINTER;
    }
    if (this->connectedto){
        if (this->connectedto == pin) {
          int accept_and_hang = 0; // hangs with poweredvd installed, doesn't affect ffdshow...hangs LAV filter
          // assume its a "graph type changed right at startup" (digital TV <cough>) which is OK
          if (accept_and_hang) {
            dshowdebug("accepting new type from upstream pin\n");
            ff_copy_dshow_media_type(&this->type, type);
            return S_OK; // XXX URLprotocol here?
          }
          dshowdebug("rejecting new type from pin\n");
          return VFW_E_ALREADY_CONNECTED; 
        } else {
          dshowdebug(" some other non connected??? d(%p)\n", this);
          return VFW_E_ALREADY_CONNECTED;
        }
    }
    if (devtype == VideoDevice) {
        // MEDIATYPE_Stream is from BDA TV infinite tee if enabled
        if ( (!IsEqualGUID(&type->majortype, &MEDIATYPE_Video)) && !IsEqualGUID(&type->majortype, &MEDIATYPE_Stream) ) {
            dshowdebug("rejecting non video and non stream stream(%p)\n", this);
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        if (IsEqualGUID(&type->formattype, &FORMAT_MPEG2_VIDEO)) {
            dshowdebug("rejecting raw mpeg2 video(%p)\n", this);
            // return not accepted here to force it to NV12 land or some decoder...i.e. not accept MPEG2VIDEO raw stream
            //return VFW_E_TYPE_NOT_ACCEPTED; // force it to insert some dshow mpeg2 converter in there for us, never could quite get that working
        }

    } else {
        if (!IsEqualGUID(&type->majortype, &MEDIATYPE_Audio)) {
            dshowdebug("rejecting non audio on audio input pin\n");
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    IPin_AddRef(pin);
    this->connectedto = pin;

    ff_copy_dshow_media_type(&this->type, type);
    dshowdebug("accepted connection at pin\n");

    return S_OK;
}
long WINAPI
libAVPin_Disconnect(libAVPin *this)
{
    dshowdebug("libAVPin_Disconnect(%p)\n", this);

    if (this->filter->state != State_Stopped) {
        dshowdebug("telling it we cannot disconnect when stopped");
        return VFW_E_NOT_STOPPED;
    }
    if (!this->connectedto) {
        dshowdebug("telling it we cannot disconnect when connected LOL");
        return S_FALSE;
    }
    IPin_Release(this->connectedto);
    this->connectedto = NULL;

    return S_OK;
}
long WINAPI
libAVPin_ConnectedTo(libAVPin *this, IPin **pin)
{
    dshowdebug("libAVPin_ConnectedTo(%p)\n", this);

    if (!pin)
        return E_POINTER;
    if (!this->connectedto) {
        dshowdebug("telling it we cannot connected toL");
        return VFW_E_NOT_CONNECTED;
    }
    IPin_AddRef(this->connectedto);
    *pin = this->connectedto;

    return S_OK;
}
long WINAPI
libAVPin_ConnectionMediaType(libAVPin *this, AM_MEDIA_TYPE *type)
{
    // they want a copy of our current media type...
    dshowdebug("libAVPin_ConnectionMediaType(%p)\n", this);

    if (!type)
        return E_POINTER;
    if (!this->connectedto) {
       dshowdebug("no con media type not connected");
       return VFW_E_NOT_CONNECTED;
    }
    
    return ff_copy_dshow_media_type(type, &this->type);
}
long WINAPI
libAVPin_QueryPinInfo(libAVPin *this, PIN_INFO *info)
{
    dshowdebug("libAVPin_QueryPinInfo(%p)\n", this);

    if (!info)
        return E_POINTER;

    if (this->filter)
        libAVFilter_AddRef(this->filter);

    info->pFilter = (IBaseFilter *) this->filter;
    info->dir     = PINDIR_INPUT;
    wcscpy(info->achName, L"Capture");

    return S_OK;
}
long WINAPI
libAVPin_QueryDirection(libAVPin *this, PIN_DIRECTION *dir)
{
    dshowdebug("libAVPin_QueryDirection(%p)\n", this);
    if (!dir)
        return E_POINTER;
    *dir = PINDIR_INPUT;
    return S_OK;
}
long WINAPI
libAVPin_QueryId(libAVPin *this, wchar_t **id)
{
    dshowdebug("libAVPin_QueryId(%p)\n", this);

    if (!id)
        return E_POINTER;

    *id = wcsdup(L"libAV Pin");

    return S_OK;
}
long WINAPI
libAVPin_QueryAccept(libAVPin *this, const AM_MEDIA_TYPE *type)
{
    int accept = 0;
    int ret;
    if (accept)  {
      dshowdebug("libAVPin_QueryAccept telling them we accept of new media offering (%p)\n", this);
      ret = S_OK;
    } else {
      dshowdebug("libAVPin_QueryAccept telling them we reject new media offering (%p)\n", this);
      ret = E_FAIL;
    }
    ff_print_AM_MEDIA_TYPE(type);
    return ret;
}
long WINAPI
libAVPin_EnumMediaTypes(libAVPin *this, IEnumMediaTypes **enumtypes)
{
    const AM_MEDIA_TYPE *type = NULL;
    libAVEnumMediaTypes *new;
    dshowdebug("libAVPin_EnumMediaTypes(%p)\n", this);

    if (!enumtypes)
        return E_POINTER;
    new = libAVEnumMediaTypes_Create(type);
    if (!new)
        return E_OUTOFMEMORY;

    *enumtypes = (IEnumMediaTypes *) new;
    return S_OK;
}
long WINAPI
libAVPin_QueryInternalConnections(libAVPin *this, IPin **pin,
                                  unsigned long *npin)
{
    dshowdebug("libAVPin_QueryInternalConnections we have none (%p)\n", this);
    return E_NOTIMPL;
}
long WINAPI
libAVPin_EndOfStream(libAVPin *this)
{
    dshowdebug("libAVPin_EndOfStream(%p)\n", this);
    /* I don't care. */
    return S_OK;
}
long WINAPI
libAVPin_BeginFlush(libAVPin *this)
{
    dshowdebug("libAVPin_BeginFlush(%p)\n", this);
    /* I don't care. */
    return S_OK;
}
long WINAPI
libAVPin_EndFlush(libAVPin *this)
{
    dshowdebug("libAVPin_EndFlush(%p)\n", this);
    /* I don't care. */
    return S_OK;
}
long WINAPI
libAVPin_NewSegment(libAVPin *this, REFERENCE_TIME start, REFERENCE_TIME stop,
                    double rate)
{
    dshowdebug("libAVPin_NewSegment(%p)\n", this);
    /* I don't care. */
    return S_OK;
}

static int
libAVPin_Setup(libAVPin *this, libAVFilter *filter)
{
    IPinVtbl *vtbl = this->vtbl;
    IMemInputPinVtbl *imemvtbl;

    if (!filter)
        return 0;

    imemvtbl = av_malloc(sizeof(IMemInputPinVtbl));
    if (!imemvtbl)
        return 0;

    SETVTBL(imemvtbl, libAVMemInputPin, QueryInterface);
    SETVTBL(imemvtbl, libAVMemInputPin, AddRef);
    SETVTBL(imemvtbl, libAVMemInputPin, Release);
    SETVTBL(imemvtbl, libAVMemInputPin, GetAllocator);
    SETVTBL(imemvtbl, libAVMemInputPin, NotifyAllocator);
    SETVTBL(imemvtbl, libAVMemInputPin, GetAllocatorRequirements);
    SETVTBL(imemvtbl, libAVMemInputPin, Receive);
    SETVTBL(imemvtbl, libAVMemInputPin, ReceiveMultiple);
    SETVTBL(imemvtbl, libAVMemInputPin, ReceiveCanBlock);

    this->imemvtbl = imemvtbl;

    SETVTBL(vtbl, libAVPin, QueryInterface); // checked it
    SETVTBL(vtbl, libAVPin, AddRef);
    SETVTBL(vtbl, libAVPin, Release);
    SETVTBL(vtbl, libAVPin, Connect);
    SETVTBL(vtbl, libAVPin, ReceiveConnection);
    SETVTBL(vtbl, libAVPin, Disconnect);
    SETVTBL(vtbl, libAVPin, ConnectedTo);
    SETVTBL(vtbl, libAVPin, ConnectionMediaType);
    SETVTBL(vtbl, libAVPin, QueryPinInfo);
    SETVTBL(vtbl, libAVPin, QueryDirection);
    SETVTBL(vtbl, libAVPin, QueryId);
    SETVTBL(vtbl, libAVPin, QueryAccept);
    SETVTBL(vtbl, libAVPin, EnumMediaTypes);
    SETVTBL(vtbl, libAVPin, QueryInternalConnections);
    SETVTBL(vtbl, libAVPin, EndOfStream);
    SETVTBL(vtbl, libAVPin, BeginFlush);
    SETVTBL(vtbl, libAVPin, EndFlush);
    SETVTBL(vtbl, libAVPin, NewSegment);

    this->filter = filter;

    return 1;
}
DECLARE_CREATE(libAVPin, libAVPin_Setup(this, filter), libAVFilter *filter)
DECLARE_DESTROY(libAVPin, nothing)

/*****************************************************************************
 * libAVMemInputPin
 ****************************************************************************/
long WINAPI
libAVMemInputPin_QueryInterface(libAVMemInputPin *this, const GUID *riid,
                                void **ppvObject)
{
    libAVPin *pin = (libAVPin *) ((uint8_t *) this - imemoffset);
    dshowdebug("libAVMemInputPin_QueryInterface(%p)\n", this);
    return libAVPin_QueryInterface(pin, riid, ppvObject);
}
unsigned long WINAPI
libAVMemInputPin_AddRef(libAVMemInputPin *this)
{
    libAVPin *pin = (libAVPin *) ((uint8_t *) this - imemoffset);
    dshowdebug("libAVMemInputPin_AddRef(%p)\n", this);
    return libAVPin_AddRef(pin);
}
unsigned long WINAPI
libAVMemInputPin_Release(libAVMemInputPin *this)
{
    libAVPin *pin = (libAVPin *) ((uint8_t *) this - imemoffset);
    dshowdebug("libAVMemInputPin_Release(%p)\n", this);
    return libAVPin_Release(pin);
}
long WINAPI
libAVMemInputPin_GetAllocator(libAVMemInputPin *this, IMemAllocator **alloc)
{
    dshowdebug("libAVMemInputPin_GetAllocator returning we have none (%p)\n", this);
    return VFW_E_NO_ALLOCATOR;
}
long WINAPI
libAVMemInputPin_NotifyAllocator(libAVMemInputPin *this, IMemAllocator *alloc,
                                 BOOL rdwr)
{
    dshowdebug("libAVMemInputPin_NotifyAllocator returning OK (%p)\n", this);
    return S_OK;
}
long WINAPI
libAVMemInputPin_GetAllocatorRequirements(libAVMemInputPin *this,
                                          ALLOCATOR_PROPERTIES *props)
{
    dshowdebug("libAVMemInputPin_GetAllocatorRequirements returning E_NOTIMPL (%p)\n", this);
    return E_NOTIMPL;
}
FILE *pFile2 = NULL;

long WINAPI
libAVMemInputPin_Receive(libAVMemInputPin *this, IMediaSample *sample)
{
    REFERENCE_TIME curtime;
    REFERENCE_TIME dummy2;
    REFERENCE_TIME orig_curtime;
    REFERENCE_TIME dummy3;
    REFERENCE_TIME graphtime;
    REFERENCE_TIME dummy4;
    libAVPin *pin = (libAVPin *) ((uint8_t *) this - imemoffset);
    enum dshowDeviceType devtype = pin->filter->type;
    void *priv_data;
    AVFormatContext *s;
    uint8_t *buf;
    int buf_size; /* todo should be a long? */
    int index;
    const char *devtypename = (devtype == VideoDevice) ? "video" : "audio";
    IReferenceClock *clock = pin->filter->clock;
    REFERENCE_TIME dummy;
    struct dshow_ctx *ctx;

    dshowdebug("libAVMemInputPin_Receive(%p)\n", this);

    if (!sample)
        return E_POINTER;
    // start time seems to work ok
    IMediaSample_GetTime(sample, &orig_curtime, &dummy);
    av_log(NULL, AV_LOG_DEBUG, "-->adding %lld to %lld sync=%d discont=%d preroll=%d\n", pin->filter->start_time, orig_curtime, IMediaSample_IsSyncPoint(sample), IMediaSample_IsDiscontinuity(sample), IMediaSample_IsPreroll(sample));
    IMediaSample_GetTime(sample, &dummy2, &dummy);
    av_log(NULL, AV_LOG_DEBUG, "adding %lld to %lld\n", pin->filter->start_time, dummy2);
    IMediaSample_GetTime(sample, &dummy3, &dummy);
    av_log(NULL, AV_LOG_DEBUG, "adding %lld to %lld\n", pin->filter->start_time, dummy3);

    orig_curtime += pin->filter->start_time;
    IReferenceClock_GetTime(clock, &graphtime);
    if (devtype == VideoDevice) {
        /* PTS from video devices is unreliable. */
        IReferenceClock_GetTime(clock, &curtime); // there is some insanity here
    } else {
        IMediaSample_GetTime(sample, &curtime, &dummy);
        av_log(NULL, AV_LOG_DEBUG, "22adding %lld to %lld\n", pin->filter->start_time, curtime);
        curtime += pin->filter->start_time;
        if(curtime > 400000000000000000LL) {
            /* initial frames sometimes start < 0 (shown as a very large number here,
               like 437650244077016960 which FFmpeg doesn't like.
               TODO figure out math. For now just drop them. */
            av_log(NULL, AV_LOG_DEBUG,
                "dshow dropping initial (or ending) audio frame with odd PTS too high %"PRId64"\n", curtime);
            return S_OK;
        }
        av_log(NULL, AV_LOG_VERBOSE, "after calculating both same  %lld ==  %lld \n", orig_curtime, curtime );
    }

    buf_size = IMediaSample_GetActualDataLength(sample);
    IMediaSample_GetPointer(sample, &buf);
    priv_data = pin->filter->priv_data;
    s = priv_data;
    ctx = s->priv_data;
    index = pin->filter->stream_index;

    av_log(NULL, AV_LOG_VERBOSE, "dshow passing through packet of type %s size %8d "
        "timestamp %lld orig timestamp %lld graph timestamp %lld diff %lld %s\n",
        devtypename, buf_size, curtime, orig_curtime, graphtime, graphtime - orig_curtime, ctx->device_name[devtype]);
    pin->filter->callback(priv_data, index, buf, buf_size, curtime, devtype);
    if (!pFile2)
       pFile2 = fopen("myfile", "ab");

    fwrite(buf, 1, buf_size, pFile2);
    //fclose(pFile2);

    return S_OK;
}
long WINAPI
libAVMemInputPin_ReceiveMultiple(libAVMemInputPin *this,
                                 IMediaSample **samples, long n, long *nproc)
{
    int i;
    dshowdebug("libAVMemInputPin_ReceiveMultiple(%p)\n", this);

    for (i = 0; i < n; i++)
        libAVMemInputPin_Receive(this, samples[i]);

    *nproc = n;
    return S_OK;
}
long WINAPI
libAVMemInputPin_ReceiveCanBlock(libAVMemInputPin *this)
{
    dshowdebug("libAVMemInputPin_ReceiveCanBlock(%p)\n", this);
    /* I swear I will not block. */
    return S_FALSE;
}

void
libAVMemInputPin_Destroy(libAVMemInputPin *this)
{
    libAVPin *pin = (libAVPin *) ((uint8_t *) this - imemoffset);
    dshowdebug("libAVMemInputPin_Destroy(%p)\n", this);
    libAVPin_Destroy(pin);
}
