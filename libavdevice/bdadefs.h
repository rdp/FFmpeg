#ifndef __BDADEFS_H
#define __BDADEFS_H

#include "tuner.h"

#ifndef __ITuner_FWD_DEFINED__
#define __ITuner_FWD_DEFINED__
typedef struct ITuner ITuner;
#endif

#ifndef __IScanningTuner_FWD_DEFINED__
#define __IScanningTuner_FWD_DEFINED__
typedef struct IScanningTuner IScanningTuner;
#endif

#ifndef __IDVBTuneRequest_FWD_DEFINED__
#define __IDVBTuneRequest_FWD_DEFINED__
typedef struct IDVBTuneRequest IDVBTuneRequest;
#endif

#ifndef __IChannelTuneRequest_FWD_DEFINED__
#define __IChannelTuneRequest_FWD_DEFINED__
typedef struct IChannelTuneRequest IChannelTuneRequest;
#endif

#ifndef __IATSCChannelTuneRequest_FWD_DEFINED__
#define __IATSCChannelTuneRequest_FWD_DEFINED__
typedef struct IATSCChannelTuneRequest IATSCChannelTuneRequest;
#endif

#ifndef __IEnumTuningSpaces_FWD_DEFINED__
#define __IEnumTuningSpaces_FWD_DEFINED__
typedef struct IEnumTuningSpaces IEnumTuningSpaces;
#endif

#ifndef __ITuningSpaces_FWD_DEFINED__
#define __ITuningSpaces_FWD_DEFINED__
typedef struct ITuningSpaces ITuningSpaces;
#endif

#ifndef __IDVBTuningSpace_FWD_DEFINED__
#define __IDVBTuningSpace_FWD_DEFINED__
typedef struct IDVBTuningSpace IDVBTuningSpace;
#endif

#ifndef __IDVBTuningSpace2_FWD_DEFINED__
#define __IDVBTuningSpace2_FWD_DEFINED__
typedef struct IDVBTuningSpace2 IDVBTuningSpace2;
#endif

#ifndef __IDVBSTuningSpace_FWD_DEFINED__
#define __IDVBSTuningSpace_FWD_DEFINED__
typedef struct IDVBSTuningSpace IDVBSTuningSpace;
#endif

#ifndef __ITuningSpaceContainer_FWD_DEFINED__
#define __ITuningSpaceContainer_FWD_DEFINED__
typedef struct ITuningSpaceContainer ITuningSpaceContainer;
#endif

#ifndef __IDVBCLocator_FWD_DEFINED__
#define __IDVBCLocator_FWD_DEFINED__
typedef struct IDVBCLocator IDVBCLocator;
#endif

#ifndef __IATSCLocator_FWD_DEFINED__
#define __IATSCLocator_FWD_DEFINED__
typedef struct IATSCLocator IATSCLocator;
#endif

EXTERN_C const GUID CLSID_SystemTuningSpaces;
EXTERN_C const IID IID_ITuningSpaceContainer;
EXTERN_C const CLSID CLSID_DVBCLocator;
EXTERN_C const CLSID CLSID_DVBTLocator;
EXTERN_C const CLSID CLSID_DVBSLocator;
EXTERN_C const CLSID CLSID_ATSCLocator;
EXTERN_C const IID IID_IDVBCLocator;
EXTERN_C const IID IID_IDVBTLocator;
EXTERN_C const IID IID_IDVBSLocator;
EXTERN_C const IID IID_IATSCLocator;
EXTERN_C const IID IID_IDVBTuningSpace;
EXTERN_C const IID IID_IDVBTuningSpace2;
EXTERN_C const IID IID_IDVBSTuningSpace;
EXTERN_C const IID IID_IATSCChannelTuneRequest;


#undef  INTERFACE
#define INTERFACE ITuner
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(ITuner,IUnknown)
{
    BEGIN_INTERFACE
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* ITuner methods */
    STDMETHOD_(HRESULT,get_TuningSpace)(THIS_ ITuningSpace** p_p_tuning_space) PURE;
    STDMETHOD_(HRESULT,put_TuningSpace)(THIS_ ITuningSpace* p_tuning_space) PURE;
    STDMETHOD_(HRESULT,EnumTuningSpaces)(THIS_ IEnumTuningSpaces** p_p_enum) PURE;
    STDMETHOD_(HRESULT,get_TuneRequest)(THIS_ ITuneRequest** p_p_tune_request) PURE;
    STDMETHOD_(HRESULT,put_TuneRequest)(THIS_ ITuneRequest* p_tune_request) PURE;
    STDMETHOD_(HRESULT,Validate)(THIS_ ITuneRequest* p_tune_request) PURE;
    STDMETHOD_(HRESULT,get_PreferredComponentTypes)(THIS_ IComponentTypes** p_p_cpt_types) PURE;
    STDMETHOD_(HRESULT,put_PreferredComponentTypes)(THIS_ IComponentTypes* p_cpt_types) PURE;
    STDMETHOD_(HRESULT,get_SignalStrength)(THIS_ long* l_sig_strength) PURE;
    STDMETHOD_(HRESULT,TriggerSignalEvents)(THIS_ long l_interval) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define ITuner_get_TuningSpace(This,p_p_tuning_space) (This)->lpVtbl->get_TuningSpace(This,p_p_tuning_space)
#define ITuner_put_TuningSpace(This,p_tuning_space) (This)->lpVtbl->put_TuningSpace(This,p_tuning_space)
#define ITuner_EnumTuningSpaces(This,p_p_enum) (This)->lpVtbl->EnumTuningSpaces(This,p_p_enum)
#define ITuner_get_TuneRequest(This,p_p_tune_request) (This)->lpVtbl->get_TuneRequest(This,p_p_tune_request)
#define ITuner_put_TuneRequest(This,p_tune_request) (This)->lpVtbl->put_TuneRequest(This,p_tune_request)
#define ITuner_Validate(This,p_tune_request) (This)->lpVtbl->Validate(This,p_tune_request)
#define ITuner_get_PreferredComponentTypes(This,p_p_cpt_types) (This)->lpVtbl->get_PreferredComponentTypes(This,p_p_cpt_types)
#define ITuner_put_PreferredComponentTypes(This,p_cpt_types) (This)->lpVtbl->put_PreferredComponentTypes(This,p_cpt_types)
#define ITuner_get_SignalStrength(This,l_sig_strength) (This)->lpVtbl->get_SignalStrength(This,l_sig_strength)
#define ITuner_TriggerSignalEvents(This,l_interval) (This)->lpVtbl->TriggerSignalEvents(This,l_interval)
#endif



#undef  INTERFACE
#define INTERFACE IDVBTuneRequest
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
  EXTERN_C const IID IID_IDVBTuneRequest;
DECLARE_INTERFACE_(IDVBTuneRequest,ITuneRequest)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuneRequest methods */
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuneRequest **ppNewTuneRequest) PURE;
    STDMETHOD_(HRESULT,get_Components)(THIS_ IComponents **ppComponents) PURE;
    STDMETHOD_(HRESULT,get_Locator)(THIS_ ILocator **ppLocator) PURE;
    STDMETHOD_(HRESULT,get_TuningSpace)(THIS_ ITuningSpace **ppTuningSpace) PURE;
    STDMETHOD_(HRESULT,put_Locator)(THIS_ ILocator *pLocator) PURE;

    /* IDVBTuneRequest methods */
    STDMETHOD_(HRESULT,get_ONID)(THIS_ long* pl_onid) PURE;
    STDMETHOD_(HRESULT,put_ONID)(THIS_ long l_onid) PURE;
    STDMETHOD_(HRESULT,get_TSID)(THIS_ long* pl_tsid) PURE;
    STDMETHOD_(HRESULT,put_TSID)(THIS_ long l_tsid) PURE;
    STDMETHOD_(HRESULT,get_SID)(THIS_ long* pl_sid) PURE;
    STDMETHOD_(HRESULT,put_SID)(THIS_ long l_sid) PURE;

    END_INTERFACE
};
#ifdef COBJMACROS
#define IDVBTuneRequest_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDVBTuneRequest_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDVBTuneRequest_Release(This) (This)->lpVtbl->Release(This)
#define IDVBTuneRequest_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDVBTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDVBTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDVBTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IDVBTuneRequest_Clone(This,ppNewTuneRequest) (This)->lpVtbl->Clone(This,ppNewTuneRequest)
#define IDVBTuneRequest_get_Components(This,ppComponents) (This)->lpVtbl->get_Components(This,ppComponents)
#define IDVBTuneRequest_get_Locator(This,ppLocator) (This)->lpVtbl->get_Locator(This,ppLocator)
#define IDVBTuneRequest_get_TuningSpace(This,ppTuningSpace) (This)->lpVtbl->get_TuningSpace(This,ppTuningSpace)
#define IDVBTuneRequest_put_Locator(This,pLocator) (This)->lpVtbl->put_Locator(This,pLocator)
#define IDVBTuneRequest_get_ONID(This,pl_onid) (This)->lpVtbl->get_ONID(This,pl_onid)
#define IDVBTuneRequest_put_ONID(This,l_onid) (This)->lpVtbl->put_ONID(This,l_onid)
#define IDVBTuneRequest_get_TSID(This,pl_tsid) (This)->lpVtbl->get_TSID(This,pl_tsid)
#define IDVBTuneRequest_put_TSID(This,l_tsid) (This)->lpVtbl->put_TSID(This,l_tsid)
#define IDVBTuneRequest_get_SID(This,pl_sid) (This)->lpVtbl->get_SID(This,pl_sid)
#define IDVBTuneRequest_put_SID(This,l_sid) (This)->lpVtbl->put_SID(This,l_sid)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IChannelTuneRequest
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IChannelTuneRequest,ITuneRequest)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuneRequest methods */
    STDMETHOD_(HRESULT,get_TuningSpace)(THIS_ ITuningSpace **ppTuningSpace) PURE;
    STDMETHOD_(HRESULT,get_Components)(THIS_ IComponents **ppComponents) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuneRequest **ppNewTuneRequest) PURE;
    STDMETHOD_(HRESULT,get_Locator)(THIS_ ILocator **ppLocator) PURE;
    STDMETHOD_(HRESULT,put_Locator)(THIS_ ILocator *pLocator) PURE;

    /* IChannelTuneRequest */
    STDMETHOD_(HRESULT,get_Channel)(THIS_ long* pl_channel) PURE;
    STDMETHOD_(HRESULT,put_Channel)(THIS_ long l_channel) PURE;


    END_INTERFACE
};
#ifdef COBJMACROS
#define IChannelTuneRequest_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IChannelTuneRequest_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IChannelTuneRequest_Release(This) (This)->lpVtbl->Release(This)
#define IChannelTuneRequest_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IChannelTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IChannelTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IChannelTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IChannelTuneRequest_get_TuningSpace(This,ppTuningSpace) (This)->lpVtbl->get_TuningSpace(This,ppTuningSpace)
#define IChannelTuneRequest_get_Components(This,ppComponents) (This)->lpVtbl->get_Components(This,ppComponents)
#define IChannelTuneRequest_Clone(This,ppNewTuneRequest) (This)->lpVtbl->Clone(This,ppNewTuneRequest)
#define IChannelTuneRequest_get_Locator(This,ppLocator) (This)->lpVtbl->get_Locator(This,ppLocator)
#define IChannelTuneRequest_put_Locator(This,pLocator) (This)->lpVtbl->put_Locator(This,pLocator)
#define IChannelTuneRequest_get_Channel(This,pl_channel) (This)->lpVtbl->get_Channel(This,pl_channel)
#define IChannelTuneRequest_put_Channel(This,l_channel) (This)->lpVtbl->put_Channel(This,l_channel)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IATSCChannelTuneRequest
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IATSCChannelTuneRequest,IChannelTuneRequest)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuneRequest methods */
    STDMETHOD_(HRESULT,get_TuningSpace)(THIS_ ITuningSpace **ppTuningSpace) PURE;
    STDMETHOD_(HRESULT,get_Components)(THIS_ IComponents **ppComponents) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuneRequest **ppNewTuneRequest) PURE;
    STDMETHOD_(HRESULT,get_Locator)(THIS_ ILocator **ppLocator) PURE;
    STDMETHOD_(HRESULT,put_Locator)(THIS_ ILocator *pLocator) PURE;

    /* IChannelTuneRequest */
    STDMETHOD_(HRESULT,get_Channel)(THIS_ long* pl_channel) PURE;
    STDMETHOD_(HRESULT,put_Channel)(THIS_ long l_channel) PURE;

    /* IATSCChannelTuneRequest */
    STDMETHOD_(HRESULT,get_MinorChannel)(THIS_ long* pl_minor_channel) PURE;
    STDMETHOD_(HRESULT,put_MinorChannel)(THIS_ long l_minor_channel) PURE;

    END_INTERFACE
};
#ifdef COBJMACROS
#define IATSCChannelTuneRequest_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IATSCChannelTuneRequest_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IATSCChannelTuneRequest_Release(This) (This)->lpVtbl->Release(This)
#define IATSCChannelTuneRequest_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IATSCChannelTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IATSCChannelTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IATSCChannelTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IATSCChannelTuneRequest_get_TuningSpace(This,ppTuningSpace) (This)->lpVtbl->get_TuningSpace(This,ppTuningSpace)
#define IATSCChannelTuneRequest_get_Components(This,ppComponents) (This)->lpVtbl->get_Components(This,ppComponents)
#define IATSCChannelTuneRequest_Clone(This,ppNewTuneRequest) (This)->lpVtbl->Clone(This,ppNewTuneRequest)
#define IATSCChannelTuneRequest_get_Locator(This,ppLocator) (This)->lpVtbl->get_Locator(This,ppLocator)
#define IATSCChannelTuneRequest_put_Locator(This,pLocator) (This)->lpVtbl->put_Locator(This,pLocator)
#define IATSCChannelTuneRequest_get_Channel(This,pl_channel) (This)->lpVtbl->get_Channel(This,pl_channel)
#define IATSCChannelTuneRequest_put_Channel(This,l_channel) (This)->lpVtbl->put_Channel(This,l_channel)
#define IATSCChannelTuneRequest_get_MinorChannel(This,pl_minor_channel) (This)->lpVtbl->get_MinorChannel(This,pl_minor_channel)
#define IATSCChannelTuneRequest_put_MinorChannel(This,l_minor_channel) (This)->lpVtbl->put_MinorChannel(This,l_minor_channel)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IScanningTuner
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
  EXTERN_C const IID IID_IScanningTuner;
DECLARE_INTERFACE_(IScanningTuner,ITuner)
{
    BEGIN_INTERFACE
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* ITuner methods */
    STDMETHOD_(HRESULT,get_TuningSpace)(THIS_ ITuningSpace** p_p_tuning_space) PURE;
    STDMETHOD_(HRESULT,put_TuningSpace)(THIS_ ITuningSpace* p_tuning_space) PURE;
    STDMETHOD_(HRESULT,EnumTuningSpaces)(THIS_ IEnumTuningSpaces** p_p_enum) PURE;
    STDMETHOD_(HRESULT,get_TuneRequest)(THIS_ ITuneRequest** p_p_tune_request) PURE;
    STDMETHOD_(HRESULT,put_TuneRequest)(THIS_ ITuneRequest* p_tune_request) PURE;
    STDMETHOD_(HRESULT,Validate)(THIS_ ITuneRequest* p_tune_request) PURE;
    STDMETHOD_(HRESULT,get_PreferredComponentTypes)(THIS_ IComponentTypes** p_p_cpt_types) PURE;
    STDMETHOD_(HRESULT,put_PreferredComponentTypes)(THIS_ IComponentTypes* p_cpt_types) PURE;
    STDMETHOD_(HRESULT,get_SignalStrength)(THIS_ long* l_sig_strength) PURE;
    STDMETHOD_(HRESULT,TriggerSignalEvents)(THIS_ long l_interval) PURE;
    
    /* IScanningTuner methods */
    STDMETHOD_(HRESULT,SeekUp)(THIS) PURE;
    STDMETHOD_(HRESULT,SeekDown)(THIS) PURE;
    STDMETHOD_(HRESULT,ScanDown)(THIS_ long l_pause) PURE;
    STDMETHOD_(HRESULT,ScanUp)(THIS_ long l_pause) PURE;
    STDMETHOD_(HRESULT,AutoProgram)(THIS) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define IScanningTuner_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IScanningTuner_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IScanningTuner_Release(This) (This)->lpVtbl->Release(This)
#define IScanningTuner_get_TuningSpace(This,p_p_tuning_space) (This)->lpVtbl->get_TuningSpace(This,p_p_tuning_space)
#define IScanningTuner_put_TuningSpace(This,p_tuning_space) (This)->lpVtbl->put_TuningSpace(This,p_tuning_space)
#define IScanningTuner_EnumTuningSpaces(This,p_p_enum) (This)->lpVtbl->EnumTuningSpaces(This,p_p_enum)
#define IScanningTuner_get_TuneRequest(This,p_p_tune_request) (This)->lpVtbl->get_TuneRequest(This,p_p_tune_request)
#define IScanningTuner_put_TuneRequest(This,p_tune_request) (This)->lpVtbl->put_TuneRequest(This,p_tune_request)
#define IScanningTuner_Validate(This,p_tune_request) (This)->lpVtbl->Validate(This,p_tune_request)
#define IScanningTuner_get_PreferredComponentTypes(This,p_p_cpt_types) (This)->lpVtbl->get_PreferredComponentTypes(This,p_p_cpt_types)
#define IScanningTuner_put_PreferredComponentTypes(This,p_cpt_types) (This)->lpVtbl->put_PreferredComponentTypes(This,p_cpt_types)
#define IScanningTuner_get_SignalStrength(This,l_sig_strength) (This)->lpVtbl->get_SignalStrength(This,l_sig_strength)
#define IScanningTuner_TriggerSignalEvents(This,l_interval) (This)->lpVtbl->TriggerSignalEvents(This,l_interval)
#define IScanningTuner_SeekUp(This) (This)->lpVtbl->SeekUp(This)
#define IScanningTuner_SeekDown(This) (This)->lpVtbl->SeekDown(This)
#define IScanningTuner_ScanDown(This, l_pause) (This)->lpVtbl->ScanDown(This, l_pause)
#define IScanningTuner_ScanUp(This, l_pause) (This)->lpVtbl->ScanUp(This, l_pause)
#define IScanningTuner_AutoProgram(This) (This)->lpVtbl->AutoProgram(This)
#endif



#undef  INTERFACE
#define INTERFACE IEnumTuningSpaces
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
  EXTERN_C const IID IID_IEnumTuningSpaces; 
DECLARE_INTERFACE_(IEnumTuningSpaces,IUnknown)
{
    BEGIN_INTERFACE
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IEnumTuningSpaces methos */
    STDMETHOD_(HRESULT,Next)(THIS_ ULONG l_num_elem,ITuningSpace **p_p_tuning_space, ULONG *pl_num_elem_fetched) PURE;
    STDMETHOD_(HRESULT,Skip)(THIS_ ULONG l_num_elem) PURE;
    STDMETHOD_(HRESULT,Reset)(THIS) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ IEnumTuningSpaces **p_p_enum ) PURE;
    END_INTERFACE
}; 
#ifdef COBJMACROS
#define IEnumTuningSpaces_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IEnumTuningSpaces_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IEnumTuningSpaces_Release(This) (This)->lpVtbl->Release(This) 
#define IEnumTuningSpaces_Next(This,l_num_elem,p_p_tuning_space,pl_num_elem_fetched) (This)->lpVtbl->Next(This,l_num_elem,p_p_tuning_space,pl_num_elem_fetched) 
#define IEnumTuningSpaces_Skip(This,l_num_elem) (This)->lpVtbl->Skip(This,l_num_elem) 
#define IEnumTuningSpaces_Reset(This) (This)->lpVtbl->Reset(This) 
#define IEnumTuningSpaces_Clone(This,p_p_enum) (This)->lpVtbl->Clone(This,p_p_enum) 
#endif



#undef  INTERFACE
#define INTERFACE ITuningSpaces
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(ITuningSpaces,IDispatch)
{
    BEGIN_INTERFACE
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,get_Count)(THIS_ LONG *l_count) PURE;
    STDMETHOD_(HRESULT,get__NewEnum)(THIS_ IEnumVARIANT **p_p_enum) PURE;
    STDMETHOD_(HRESULT,get_Item)(THIS_ VARIANT v_index, ITuningSpace **p_p_tuning_space) PURE;
    STDMETHOD_(HRESULT,get_EnumTuningSpaces)(THIS_ IEnumTuningSpaces **p_p_enum) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define ITuningSpaces_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITuningSpaces_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITuningSpaces_Release(This) (This)->lpVtbl->Release(This)
#define ITuningSpaces_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITuningSpaces_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITuningSpaces_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITuningSpaces_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) 
#define ITuningSpaces_get_Count(This,l_count) (This)->lpVtbl->get_Count(This,l_count)
#define ITuningSpaces_get__NewEnum(This,p_p_enum) (This)->lpVtbl->get__NewEnum(This,p_p_enum)
#define ITuningSpaces_get_Item(This,v_index,p_p_tuning_space) (This)->lpVtbl->get_Item(This,v_index,p_p_tuning_space)
#define ITuningSpaces_get_EnumTuningSpaces(This,p_p_enum) (This)->lpVtbl->put_Item(This,p_p_enum)
#endif



#undef  INTERFACE
#define INTERFACE ITuningSpaceContainer
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(ITuningSpaceContainer,IDispatch)
{
    BEGIN_INTERFACE
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuningSpaceContainer methods */
    STDMETHOD_(HRESULT,get_Count)(THIS_ LONG *l_count) PURE;
    STDMETHOD_(HRESULT,get__NewEnum)(THIS_ IEnumVARIANT **p_p_enum) PURE;
    STDMETHOD_(HRESULT,get_Item)(THIS_ VARIANT v_index, ITuningSpace **p_p_tuning_space) PURE;
    STDMETHOD_(HRESULT,put_Item)(THIS_ VARIANT v_index, ITuningSpace *p_tuning_space) PURE;
    STDMETHOD_(HRESULT,TuningSpacesForCLSID)(THIS_ BSTR bstr_clsid, ITuningSpaces **p_p_tuning_spaces) PURE;
    STDMETHOD_(HRESULT,_TuningSpacesForCLSID)(THIS_ REFCLSID clsid, ITuningSpaces **p_p_tuning_spaces) PURE;
    STDMETHOD_(HRESULT,TuningSpacesForName)(THIS_ BSTR bstr_name, ITuningSpaces **p_p_tuning_spaces) PURE;
    STDMETHOD_(HRESULT,FindID)(THIS_ ITuningSpace *p_tuning_space, LONG *l_id) PURE;
    STDMETHOD_(HRESULT,Add)(THIS_ ITuningSpace *p_tuning_space, VARIANT *v_index) PURE;
    STDMETHOD_(HRESULT,get_EnumTuningSpaces)(THIS_ IEnumTuningSpaces **p_p_enum) PURE;
    STDMETHOD_(HRESULT,Remove)(THIS_ VARIANT v_index) PURE;
    STDMETHOD_(HRESULT,get_MaxCount)(THIS_ LONG *l_maxcount) PURE;
    STDMETHOD_(HRESULT,put_MaxCount)(THIS_ LONG l_maxcount) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define ITuningSpaceContainer_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define ITuningSpaceContainer_AddRef(This) (This)->lpVtbl->AddRef(This)
#define ITuningSpaceContainer_Release(This) (This)->lpVtbl->Release(This)
#define ITuningSpaceContainer_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define ITuningSpaceContainer_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define ITuningSpaceContainer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define ITuningSpaceContainer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) 
#define ITuningSpaceContainer_get_Count(This,l_count) (This)->lpVtbl->get_Count(This,l_count)
#define ITuningSpaceContainer_get__NewEnum(This,p_p_enum) (This)->lpVtbl->get__NewEnum(This,p_p_enum)
#define ITuningSpaceContainer_get_Item(This) (This)->lpVtbl->get_Item(This,v_index,p_p_tuning_space)
#define ITuningSpaceContainer_put_Item(This,v_index,p_tuning_space) (This)->lpVtbl->put_Item(This,v_index,p_tuning_space)
#define ITuningSpaceContainer_TuningSpacesForCLSID(This,bstr_clsid,p_p_tuning_spaces) (This)->lpVtbl->TuningSpacesForCLSID(This,bstr_clsid,p_p_tuning_spaces)
#define ITuningSpaceContainer__TuningSpacesForCLSID(This,clsid,p_p_tuning_spaces) (This)->lpVtbl->_TuningSpacesForCLSID(This,clsid,p_p_tuning_spaces)
#define ITuningSpaceContainer_TuningSpacesForName(This,bstr_name,p_p_tuning_spaces) (This)->lpVtbl->TuningSpacesForName(This,bstr_name,p_p_tuning_spaces)
#define ITuningSpaceContainer_FindID(This,p_tuning_space,l_id) (This)->lpVtbl->FindID(This,p_tuning_space,l_id)
#define ITuningSpaceContainer_Add(This,p_tuning_space,v_index) (This)->lpVtbl->Add(This,p_tuning_space,v_index)
#define ITuningSpaceContainer_get_EnumTuningSpaces(This,p_p_enum) (This)->lpVtbl->get_EnumTuningSpaces(This,p_p_enum)
#define ITuningSpaceContainer_Remove(This,v_index) (This)->lpVtbl->Remove(This,v_index)
#define ITuningSpaceContainer_get_MaxCount(This,l_maxcount) (This)->lpVtbl->get_MaxCount(This,l_maxcount)
#define ITuningSpaceContainer_put_MaxCount(This,l_maxcount) (This)->lpVtbl->put_MaxCount(This,l_maxcount)
#endif



#undef  INTERFACE
#define INTERFACE IDVBTuningSpace
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IDVBTuningSpace,ITuningSpace)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuningSpace methods */
    STDMETHOD_(HRESULT,get_UniqueName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_UniqueName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_FriendlyName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_FriendlyName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_CLSID)(THIS_ BSTR *pSpaceCLSID) PURE;
    STDMETHOD_(HRESULT,get_NetworkType)(THIS_ BSTR *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put_NetworkType)(THIS_ BSTR NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,get__NetworkType)(THIS_ GUID *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put__NetworkType)(THIS_ REFCLSID NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,CreateTuneRequest)(THIS_ ITuneRequest **ppTuneRequest) PURE;
    STDMETHOD_(HRESULT,EnumCategoryGUIDs)(THIS_ IEnumGUID **ppEnum) PURE;
    STDMETHOD_(HRESULT,EnumDeviceMonikers)(THIS_ IEnumMoniker **ppEnum) PURE;
    STDMETHOD_(HRESULT,get_DefaultPreferredComponentTypes)(THIS_ IComponentTypes **ppComponentTypes) PURE;
    STDMETHOD_(HRESULT,put_DefaultPreferredComponentTypes)(THIS_ IComponentTypes *pNewComponentTypes) PURE;
    STDMETHOD_(HRESULT,get_FrequencyMapping)(THIS_ BSTR *pMapping) PURE;
    STDMETHOD_(HRESULT,put_FrequencyMapping)(THIS_ BSTR Mapping) PURE;
    STDMETHOD_(HRESULT,get_DefaultLocator)(THIS_ ILocator **ppLocatorVal) PURE;
    STDMETHOD_(HRESULT,put_DefaultLocator)(THIS_ ILocator *pLocatorVal) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuningSpace **ppNewTS) PURE;

    /* IDVBTuningSpace methods */
    STDMETHOD_(HRESULT,get_SystemType)(THIS_ DVBSystemType* p_sys_type) PURE;
    STDMETHOD_(HRESULT,put_SystemType)(THIS_ DVBSystemType sys_type) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define IDVBTuningSpace_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDVBTuningSpace_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDVBTuningSpace_Release(This) (This)->lpVtbl->Release(This)
#define IDVBTuningSpace_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDVBTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDVBTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDVBTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IDVBTuningSpace_get_UniqueName(This,pName) (This)->lpVtbl->get_UniqueName(This,pName)
#define IDVBTuningSpace_put_UniqueName(This,Name) (This)->lpVtbl->put_UniqueName(This,Name)
#define IDVBTuningSpace_get_FriendlyName(This,pName) (This)->lpVtbl->get_FriendlyName(This,pName)
#define IDVBTuningSpace_put_FriendlyName(This,Name) (This)->lpVtbl->put_FriendlyName(This,Name)
#define IDVBTuningSpace_get_CLSID(This,pSpaceCLSID) (This)->lpVtbl->get_CLSID(This,pSpaceCLSID)
#define IDVBTuningSpace_get_NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get_NetworkType(This,pNetworkTypeGuid)
#define IDVBTuningSpace_put_NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put_NetworkType(This,NetworkTypeGuid)
#define IDVBTuningSpace_get__NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get__NetworkType(This,pNetworkTypeGuid)
#define IDVBTuningSpace_put__NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put__NetworkType(This,NetworkTypeGuid)
#define IDVBTuningSpace_CreateTuneRequest(This,ppTuneRequest) (This)->lpVtbl->CreateTuneRequest(This,ppTuneRequest)
#define IDVBTuningSpace_EnumCategoryGUIDs(This,ppEnum) (This)->lpVtbl->EnumCategoryGUIDs(This,ppEnum)
#define IDVBTuningSpace_EnumDeviceMonikers(This,ppEnum) (This)->lpVtbl->EnumDeviceMonikers(This,ppEnum)
#define IDVBTuningSpace_get_DefaultPreferredComponentTypes(This,ppComponentTypes) (This)->lpVtbl->get_DefaultPreferredComponentTypes(This,ppComponentTypes)
#define IDVBTuningSpace_put_DefaultPreferredComponentTypes(This,pNewComponentTypes) (This)->lpVtbl->put_DefaultPreferredComponentTypes(This,pNewComponentTypes)
#define IDVBTuningSpace_get_FrequencyMapping(This,pMapping) (This)->lpVtbl->get_FrequencyMapping(This,pMapping)
#define IDVBTuningSpace_put_FrequencyMapping(This,Mapping) (This)->lpVtbl->put_FrequencyMapping(This,Mapping)
#define IDVBTuningSpace_get_DefaultLocator(This,ppLocatorVal) (This)->lpVtbl->get_DefaultLocator(This,ppLocatorVal)
#define IDVBTuningSpace_put_DefaultLocator(This,pLocatorVal) (This)->lpVtbl->put_DefaultLocator(This,pLocatorVal)
#define IDVBTuningSpace_Clone(This,ppNewTS) (This)->lpVtbl->Clone(This,ppNewTS)
#define IDVBTuningSpace_get_SystemType(This,p_sys_type) (This)->lpVtbl->get_SystemType(This,p_sys_type)
#define IDVBTuningSpace_put_SystemType(This,sys_type) (This)->lpVtbl->put_SystemType(This,sys_type)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IDVBTuningSpace2
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IDVBTuningSpace2,IDVBTuningSpace)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuningSpace methods */
    STDMETHOD_(HRESULT,get_UniqueName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_UniqueName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_FriendlyName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_FriendlyName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_CLSID)(THIS_ BSTR *pSpaceCLSID) PURE;
    STDMETHOD_(HRESULT,get_NetworkType)(THIS_ BSTR *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put_NetworkType)(THIS_ BSTR NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,get__NetworkType)(THIS_ GUID *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put__NetworkType)(THIS_ REFCLSID NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,CreateTuneRequest)(THIS_ ITuneRequest **ppTuneRequest) PURE;
    STDMETHOD_(HRESULT,EnumCategoryGUIDs)(THIS_ IEnumGUID **ppEnum) PURE;
    STDMETHOD_(HRESULT,EnumDeviceMonikers)(THIS_ IEnumMoniker **ppEnum) PURE;
    STDMETHOD_(HRESULT,get_DefaultPreferredComponentTypes)(THIS_ IComponentTypes **ppComponentTypes) PURE;
    STDMETHOD_(HRESULT,put_DefaultPreferredComponentTypes)(THIS_ IComponentTypes *pNewComponentTypes) PURE;
    STDMETHOD_(HRESULT,get_FrequencyMapping)(THIS_ BSTR *pMapping) PURE;
    STDMETHOD_(HRESULT,put_FrequencyMapping)(THIS_ BSTR Mapping) PURE;
    STDMETHOD_(HRESULT,get_DefaultLocator)(THIS_ ILocator **ppLocatorVal) PURE;
    STDMETHOD_(HRESULT,put_DefaultLocator)(THIS_ ILocator *pLocatorVal) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuningSpace **ppNewTS) PURE;

    /* IDVBTuningSpace methods */
    STDMETHOD_(HRESULT,get_SystemType)(THIS_ DVBSystemType* p_sys_type) PURE;
    STDMETHOD_(HRESULT,put_SystemType)(THIS_ DVBSystemType sys_type) PURE;

    /* IDVBTuningSpace2 methods */
    STDMETHOD_(HRESULT,get_NetworkID)(THIS_ long* p_l_network_id) PURE;
    STDMETHOD_(HRESULT,put_NetworkID)(THIS_ long l_network_id) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define IDVBTuningSpace2_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDVBTuningSpace2_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDVBTuningSpace2_Release(This) (This)->lpVtbl->Release(This)
#define IDVBTuningSpace2_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDVBTuningSpace2_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDVBTuningSpace2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDVBTuningSpace2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IDVBTuningSpace2_get_UniqueName(This,pName) (This)->lpVtbl->get_UniqueName(This,pName)
#define IDVBTuningSpace2_put_UniqueName(This,Name) (This)->lpVtbl->put_UniqueName(This,Name)
#define IDVBTuningSpace2_get_FriendlyName(This,pName) (This)->lpVtbl->get_FriendlyName(This,pName)
#define IDVBTuningSpace2_put_FriendlyName(This,Name) (This)->lpVtbl->put_FriendlyName(This,Name)
#define IDVBTuningSpace2_get_CLSID(This,pSpaceCLSID) (This)->lpVtbl->get_CLSID(This,pSpaceCLSID)
#define IDVBTuningSpace2_get_NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get_NetworkType(This,pNetworkTypeGuid)
#define IDVBTuningSpace2_put_NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put_NetworkType(This,NetworkTypeGuid)
#define IDVBTuningSpace2_get__NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get__NetworkType(This,pNetworkTypeGuid)
#define IDVBTuningSpace2_put__NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put__NetworkType(This,NetworkTypeGuid)
#define IDVBTuningSpace2_CreateTuneRequest(This,ppTuneRequest) (This)->lpVtbl->CreateTuneRequest(This,ppTuneRequest)
#define IDVBTuningSpace2_EnumCategoryGUIDs(This,ppEnum) (This)->lpVtbl->EnumCategoryGUIDs(This,ppEnum)
#define IDVBTuningSpace2_EnumDeviceMonikers(This,ppEnum) (This)->lpVtbl->EnumDeviceMonikers(This,ppEnum)
#define IDVBTuningSpace2_get_DefaultPreferredComponentTypes(This,ppComponentTypes) (This)->lpVtbl->get_DefaultPreferredComponentTypes(This,ppComponentTypes)
#define IDVBTuningSpace2_put_DefaultPreferredComponentTypes(This,pNewComponentTypes) (This)->lpVtbl->put_DefaultPreferredComponentTypes(This,pNewComponentTypes)
#define IDVBTuningSpace2_get_FrequencyMapping(This,pMapping) (This)->lpVtbl->get_FrequencyMapping(This,pMapping)
#define IDVBTuningSpace2_put_FrequencyMapping(This,Mapping) (This)->lpVtbl->put_FrequencyMapping(This,Mapping)
#define IDVBTuningSpace2_get_DefaultLocator(This,ppLocatorVal) (This)->lpVtbl->get_DefaultLocator(This,ppLocatorVal)
#define IDVBTuningSpace2_put_DefaultLocator(This,pLocatorVal) (This)->lpVtbl->put_DefaultLocator(This,pLocatorVal)
#define IDVBTuningSpace2_Clone(This,ppNewTS) (This)->lpVtbl->Clone(This,ppNewTS)
#define IDVBTuningSpace2_get_SystemType(This,p_sys_type) (This)->lpVtbl->get_SystemType(This,p_sys_type)
#define IDVBTuningSpace2_put_SystemType(This,sys_type) (This)->lpVtbl->put_SystemType(This,sys_type)
#define IDVBTuningSpace2_get_NetworkID(This,p_l_network_id) (This)->lpVtbl->get_NetworkID(This,p_l_network_id)
#define IDVBTuningSpace2_put_NetworkID(This,l_network_id) (This)->lpVtbl->put_NetworkID(This,l_network_id)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IDVBSTuningSpace
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IDVBSTuningSpace,IDVBTuningSpace2)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ITuningSpace methods */
    STDMETHOD_(HRESULT,get_UniqueName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_UniqueName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_FriendlyName)(THIS_ BSTR *pName) PURE;
    STDMETHOD_(HRESULT,put_FriendlyName)(THIS_ BSTR Name) PURE;
    STDMETHOD_(HRESULT,get_CLSID)(THIS_ BSTR *pSpaceCLSID) PURE;
    STDMETHOD_(HRESULT,get_NetworkType)(THIS_ BSTR *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put_NetworkType)(THIS_ BSTR NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,get__NetworkType)(THIS_ GUID *pNetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,put__NetworkType)(THIS_ REFCLSID NetworkTypeGuid) PURE;
    STDMETHOD_(HRESULT,CreateTuneRequest)(THIS_ ITuneRequest **ppTuneRequest) PURE;
    STDMETHOD_(HRESULT,EnumCategoryGUIDs)(THIS_ IEnumGUID **ppEnum) PURE;
    STDMETHOD_(HRESULT,EnumDeviceMonikers)(THIS_ IEnumMoniker **ppEnum) PURE;
    STDMETHOD_(HRESULT,get_DefaultPreferredComponentTypes)(THIS_ IComponentTypes **ppComponentTypes) PURE;
    STDMETHOD_(HRESULT,put_DefaultPreferredComponentTypes)(THIS_ IComponentTypes *pNewComponentTypes) PURE;
    STDMETHOD_(HRESULT,get_FrequencyMapping)(THIS_ BSTR *pMapping) PURE;
    STDMETHOD_(HRESULT,put_FrequencyMapping)(THIS_ BSTR Mapping) PURE;
    STDMETHOD_(HRESULT,get_DefaultLocator)(THIS_ ILocator **ppLocatorVal) PURE;
    STDMETHOD_(HRESULT,put_DefaultLocator)(THIS_ ILocator *pLocatorVal) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ ITuningSpace **ppNewTS) PURE;

    /* IDVBTuningSpace methods */
    STDMETHOD_(HRESULT,get_SystemType)(THIS_ DVBSystemType* p_sys_type) PURE;
    STDMETHOD_(HRESULT,put_SystemType)(THIS_ DVBSystemType sys_type) PURE;

    /* IDVBTuningSpace2 methods */
    STDMETHOD_(HRESULT,get_NetworkID)(THIS_ long* p_l_network_id) PURE;
    STDMETHOD_(HRESULT,put_NetworkID)(THIS_ long l_network_id) PURE;

    /* IDVBSTuningSpace methods */
    STDMETHOD_(HRESULT,get_LowOscillator)(THIS_ long* p_l_low_osc) PURE;
    STDMETHOD_(HRESULT,put_LowOscillator)(THIS_ long l_low_osc) PURE;
    STDMETHOD_(HRESULT,get_HighOscillator)(THIS_ long* p_l_high_osc) PURE;
    STDMETHOD_(HRESULT,put_HighOscillator)(THIS_ long l_high_osc) PURE;
    STDMETHOD_(HRESULT,get_LNBSwitch)(THIS_ long* p_l_lnb_switch) PURE;
    STDMETHOD_(HRESULT,put_LNBSwitch)(THIS_ long l_lnb_switch) PURE;
    STDMETHOD_(HRESULT,get_InputRange)(THIS_ BSTR* p_bstr_input_range) PURE;
    STDMETHOD_(HRESULT,put_InputRange)(THIS_ BSTR bstr_input_range) PURE;
    STDMETHOD_(HRESULT,get_SpectralInversion)(THIS_ SpectralInversion* p_spectral_inv) PURE;
    STDMETHOD_(HRESULT,put_SpectralInversion)(THIS_ SpectralInversion spectral_inv) PURE;
    END_INTERFACE
};
#ifdef COBJMACROS
#define IDVBSTuningSpace_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDVBSTuningSpace_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDVBSTuningSpace_Release(This) (This)->lpVtbl->Release(This)
#define IDVBSTuningSpace_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDVBSTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDVBSTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDVBSTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IDVBSTuningSpace_get_UniqueName(This,pName) (This)->lpVtbl->get_UniqueName(This,pName)
#define IDVBSTuningSpace_put_UniqueName(This,Name) (This)->lpVtbl->put_UniqueName(This,Name)
#define IDVBSTuningSpace_get_FriendlyName(This,pName) (This)->lpVtbl->get_FriendlyName(This,pName)
#define IDVBSTuningSpace_put_FriendlyName(This,Name) (This)->lpVtbl->put_FriendlyName(This,Name)
#define IDVBSTuningSpace_get_CLSID(This,pSpaceCLSID) (This)->lpVtbl->get_CLSID(This,pSpaceCLSID)
#define IDVBSTuningSpace_get_NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get_NetworkType(This,pNetworkTypeGuid)
#define IDVBSTuningSpace_put_NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put_NetworkType(This,NetworkTypeGuid)
#define IDVBSTuningSpace_get__NetworkType(This,pNetworkTypeGuid) (This)->lpVtbl->get__NetworkType(This,pNetworkTypeGuid)
#define IDVBSTuningSpace_put__NetworkType(This,NetworkTypeGuid) (This)->lpVtbl->put__NetworkType(This,NetworkTypeGuid)
#define IDVBSTuningSpace_CreateTuneRequest(This,ppTuneRequest) (This)->lpVtbl->CreateTuneRequest(This,ppTuneRequest)
#define IDVBSTuningSpace_EnumCategoryGUIDs(This,ppEnum) (This)->lpVtbl->EnumCategoryGUIDs(This,ppEnum)
#define IDVBSTuningSpace_EnumDeviceMonikers(This,ppEnum) (This)->lpVtbl->EnumDeviceMonikers(This,ppEnum)
#define IDVBSTuningSpace_get_DefaultPreferredComponentTypes(This,ppComponentTypes) (This)->lpVtbl->get_DefaultPreferredComponentTypes(This,ppComponentTypes)
#define IDVBSTuningSpace_put_DefaultPreferredComponentTypes(This,pNewComponentTypes) (This)->lpVtbl->put_DefaultPreferredComponentTypes(This,pNewComponentTypes)
#define IDVBSTuningSpace_get_FrequencyMapping(This,pMapping) (This)->lpVtbl->get_FrequencyMapping(This,pMapping)
#define IDVBSTuningSpace_put_FrequencyMapping(This,Mapping) (This)->lpVtbl->put_FrequencyMapping(This,Mapping)
#define IDVBSTuningSpace_get_DefaultLocator(This,ppLocatorVal) (This)->lpVtbl->get_DefaultLocator(This,ppLocatorVal)
#define IDVBSTuningSpace_put_DefaultLocator(This,pLocatorVal) (This)->lpVtbl->put_DefaultLocator(This,pLocatorVal)
#define IDVBSTuningSpace_Clone(This,ppNewTS) (This)->lpVtbl->Clone(This,ppNewTS)
#define IDVBSTuningSpace_get_SystemType(This,p_sys_type) (This)->lpVtbl->get_SystemType(This,p_sys_type)
#define IDVBSTuningSpace_put_SystemType(This,sys_type) (This)->lpVtbl->put_SystemType(This,sys_type)
#define IDVBSTuningSpace_get_NetworkID(This,p_l_network_id) (This)->lpVtbl->get_NetworkID(This,p_l_network_id)
#define IDVBSTuningSpace_put_NetworkID(This,l_network_id) (This)->lpVtbl->put_NetworkID(This,l_network_id)
#define IDVBSTuningSpace_get_LowOscillator(This,p_l_low_osc) (This)->lpVtbl->get_LowOscillator(This,p_l_low_osc)
#define IDVBSTuningSpace_put_LowOscillator(This,l_low_osc) (This)->lpVtbl->put_LowOscillator(This,l_low_osc)
#define IDVBSTuningSpace_get_HighOscillator(This,p_l_high_osc) (This)->lpVtbl->get_HighOscillator(This,p_l_high_osc)
#define IDVBSTuningSpace_put_HighOscillator(This,l_high_osc) (This)->lpVtbl->put_HighOscillator(This,l_high_osc)
#define IDVBSTuningSpace_get_LNBSwitch(This,p_l_lnb_switch) (This)->lpVtbl->get_LNBSwitch(This,p_l_lnb_switch)
#define IDVBSTuningSpace_put_LNBSwitch(This,l_lnb_switch) (This)->lpVtbl->put_LNBSwitch(This,l_lnb_switch)
#define IDVBSTuningSpace_get_InputRange(This,p_bstr_input_range) (This)->lpVtbl->get_InputRange(This,p_bstr_input_range)
#define IDVBSTuningSpace_put_InputRange(This,bstr_input_range) (This)->lpVtbl->put_InputRange(This,bstr_input_range)
#define IDVBSTuningSpace_get_SpectralInversion(This,p_spectral_inv) (This)->lpVtbl->get_SpectralInversion(This,p_spectral_inv)
#define IDVBSTuningSpace_put_SpectralInversion(This,spectral_inv) (This)->lpVtbl->put_SpectralInversion(This,spectral_inv)
#endif /*COBJMACROS*/



#undef  INTERFACE
#define INTERFACE IDVBCLocator
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IDVBCLocator,ILocator)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ILocator methods */
    STDMETHOD_(HRESULT,get_CarrierFrequency)(THIS_ __LONG32 *pFrequency) PURE;
    STDMETHOD_(HRESULT,put_CarrierFrequency)(THIS_ __LONG32 Frequency) PURE;
    STDMETHOD_(HRESULT,get_InnerFEC)(THIS_ FECMethod *FEC) PURE;
    STDMETHOD_(HRESULT,put_InnerFEC)(THIS_ FECMethod FEC) PURE;
    STDMETHOD_(HRESULT,get_InnerFECRate)(THIS_ BinaryConvolutionCodeRate *FEC) PURE;
    STDMETHOD_(HRESULT,put_InnerFECRate)(THIS_ BinaryConvolutionCodeRate FEC) PURE;
    STDMETHOD_(HRESULT,get_OuterFEC)(THIS_ FECMethod *FEC) PURE;
    STDMETHOD_(HRESULT,put_OuterFEC)(THIS_ FECMethod FEC) PURE;
    STDMETHOD_(HRESULT,get_OuterFECRate)(THIS_ BinaryConvolutionCodeRate *FEC) PURE;
    STDMETHOD_(HRESULT,put_OuterFECRate)(THIS_ BinaryConvolutionCodeRate FEC) PURE;
    STDMETHOD_(HRESULT,get_Modulation)(THIS_ ModulationType *pModulation) PURE;
    STDMETHOD_(HRESULT,put_Modulation)(THIS_ ModulationType Modulation) PURE;
    STDMETHOD_(HRESULT,get_SymbolRate)(THIS_ __LONG32 *Rate) PURE;
    STDMETHOD_(HRESULT,put_SymbolRate)(THIS_ __LONG32 Rate) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ IDVBCLocator **ppNewLocator) PURE;

    /* IDVBCLocator methods */

    END_INTERFACE
};
#ifdef COBJMACROS
#define IDVBCLocator_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IDVBCLocator_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IDVBCLocator_Release(This) (This)->lpVtbl->Release(This)
#define IDVBCLocator_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IDVBCLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IDVBCLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IDVBCLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IDVBCLocator_Clone(This,ppNewLocator) (This)->lpVtbl->Clone(This,ppNewLocator)
#define IDVBCLocator_get_CarrierFrequency(This,pFrequency) (This)->lpVtbl->get_CarrierFrequency(This,pFrequency)
#define IDVBCLocator_get_InnerFEC(This,FEC) (This)->lpVtbl->get_InnerFEC(This,FEC)
#define IDVBCLocator_get_InnerFECRate(This,FEC) (This)->lpVtbl->get_InnerFECRate(This,FEC)
#define IDVBCLocator_get_Modulation(This,pModulation) (This)->lpVtbl->get_Modulation(This,pModulation)
#define IDVBCLocator_get_OuterFEC(This,FEC) (This)->lpVtbl->get_OuterFEC(This,FEC)
#define IDVBCLocator_get_OuterFECRate(This,FEC) (This)->lpVtbl->get_OuterFECRate(This,FEC)
#define IDVBCLocator_get_SymbolRate(This,Rate) (This)->lpVtbl->get_SymbolRate(This,Rate)
#define IDVBCLocator_put_CarrierFrequency(This,Frequency) (This)->lpVtbl->put_CarrierFrequency(This,Frequency)
#define IDVBCLocator_put_InnerFEC(This,FEC) (This)->lpVtbl->put_InnerFEC(This,FEC)
#define IDVBCLocator_put_InnerFECRate(This,FEC) (This)->lpVtbl->put_InnerFECRate(This,FEC)
#define IDVBCLocator_put_Modulation(This,Modulation) (This)->lpVtbl->put_Modulation(This,Modulation)
#define IDVBCLocator_put_OuterFEC(This,FEC) (This)->lpVtbl->put_OuterFEC(This,FEC)
#define IDVBCLocator_put_OuterFECRate(This,FEC) (This)->lpVtbl->put_OuterFECRate(This,FEC)
#define IDVBCLocator_put_SymbolRate(This,Rate) (This)->lpVtbl->put_SymbolRate(This,Rate)
#endif /*COBJMACROS*/




#undef  INTERFACE
#define INTERFACE IATSCLocator
#ifdef __GNUC__
///#warning COM interfaces layout in this header has not been verified.
///#warning COM interfaces with incorrect layout may not work at all.
///__MINGW_BROKEN_INTERFACE(INTERFACE)
#endif
DECLARE_INTERFACE_(IATSCLocator,ILocator)
{
    BEGIN_INTERFACE

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    /* IDispatch methods */
    STDMETHOD_(HRESULT,GetTypeInfoCount)(THIS_ UINT *pctinfo) PURE;
    STDMETHOD_(HRESULT,GetTypeInfo)(THIS_ UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) PURE;
    STDMETHOD_(HRESULT,GetIDsOfNames)(THIS_ REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) PURE;
    STDMETHOD_(HRESULT,Invoke)(THIS_ DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) PURE;

    /* ILocator methods */
    STDMETHOD_(HRESULT,get_CarrierFrequency)(THIS_ __LONG32 *pFrequency) PURE;
    STDMETHOD_(HRESULT,put_CarrierFrequency)(THIS_ __LONG32 Frequency) PURE;
    STDMETHOD_(HRESULT,get_InnerFEC)(THIS_ FECMethod *FEC) PURE;
    STDMETHOD_(HRESULT,put_InnerFEC)(THIS_ FECMethod FEC) PURE;
    STDMETHOD_(HRESULT,get_InnerFECRate)(THIS_ BinaryConvolutionCodeRate *FEC) PURE;
    STDMETHOD_(HRESULT,put_InnerFECRate)(THIS_ BinaryConvolutionCodeRate FEC) PURE;
    STDMETHOD_(HRESULT,get_OuterFEC)(THIS_ FECMethod *FEC) PURE;
    STDMETHOD_(HRESULT,put_OuterFEC)(THIS_ FECMethod FEC) PURE;
    STDMETHOD_(HRESULT,get_OuterFECRate)(THIS_ BinaryConvolutionCodeRate *FEC) PURE;
    STDMETHOD_(HRESULT,put_OuterFECRate)(THIS_ BinaryConvolutionCodeRate FEC) PURE;
    STDMETHOD_(HRESULT,get_Modulation)(THIS_ ModulationType *pModulation) PURE;
    STDMETHOD_(HRESULT,put_Modulation)(THIS_ ModulationType Modulation) PURE;
    STDMETHOD_(HRESULT,get_SymbolRate)(THIS_ __LONG32 *Rate) PURE;
    STDMETHOD_(HRESULT,put_SymbolRate)(THIS_ __LONG32 Rate) PURE;
    STDMETHOD_(HRESULT,Clone)(THIS_ IDVBCLocator **ppNewLocator) PURE;

    /* IATSCLocator methods */
    STDMETHOD_(HRESULT,get_PhysicalChannel)(THIS_ long* pl_phys_channel) PURE;
    STDMETHOD_(HRESULT,put_PhysicalChannel)(THIS_ long l_phys_channel) PURE;
    STDMETHOD_(HRESULT,get_TSID)(THIS_ long* pl_tsid) PURE;
    STDMETHOD_(HRESULT,put_TSID)(THIS_ long l_tsid) PURE;

    END_INTERFACE
};
#ifdef COBJMACROS
#define IATSCLocator_QueryInterface(This,riid,ppvObject) (This)->lpVtbl->QueryInterface(This,riid,ppvObject)
#define IATSCLocator_AddRef(This) (This)->lpVtbl->AddRef(This)
#define IATSCLocator_Release(This) (This)->lpVtbl->Release(This)
#define IATSCLocator_GetTypeInfoCount(This,pctinfo) (This)->lpVtbl->GetTypeInfoCount(This,pctinfo)
#define IATSCLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo) (This)->lpVtbl->GetTypeInfo(This,iTInfo,lcid,ppTInfo)
#define IATSCLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) (This)->lpVtbl->GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)
#define IATSCLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) (This)->lpVtbl->Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)
#define IATSCLocator_Clone(This,ppNewLocator) (This)->lpVtbl->Clone(This,ppNewLocator)
#define IATSCLocator_get_CarrierFrequency(This,pFrequency) (This)->lpVtbl->get_CarrierFrequency(This,pFrequency)
#define IATSCLocator_get_InnerFEC(This,FEC) (This)->lpVtbl->get_InnerFEC(This,FEC)
#define IATSCLocator_get_InnerFECRate(This,FEC) (This)->lpVtbl->get_InnerFECRate(This,FEC)
#define IATSCLocator_get_Modulation(This,pModulation) (This)->lpVtbl->get_Modulation(This,pModulation)
#define IATSCLocator_get_OuterFEC(This,FEC) (This)->lpVtbl->get_OuterFEC(This,FEC)
#define IATSCLocator_get_OuterFECRate(This,FEC) (This)->lpVtbl->get_OuterFECRate(This,FEC)
#define IATSCLocator_get_SymbolRate(This,Rate) (This)->lpVtbl->get_SymbolRate(This,Rate)
#define IATSCLocator_put_CarrierFrequency(This,Frequency) (This)->lpVtbl->put_CarrierFrequency(This,Frequency)
#define IATSCLocator_put_InnerFEC(This,FEC) (This)->lpVtbl->put_InnerFEC(This,FEC)
#define IATSCLocator_put_InnerFECRate(This,FEC) (This)->lpVtbl->put_InnerFECRate(This,FEC)
#define IATSCLocator_put_Modulation(This,Modulation) (This)->lpVtbl->put_Modulation(This,Modulation)
#define IATSCLocator_put_OuterFEC(This,FEC) (This)->lpVtbl->put_OuterFEC(This,FEC)
#define IATSCLocator_put_OuterFECRate(This,FEC) (This)->lpVtbl->put_OuterFECRate(This,FEC)
#define IATSCLocator_put_SymbolRate(This,Rate) (This)->lpVtbl->put_SymbolRate(This,Rate)
#define IATSCLocator_get_PhysicalChannel(This,pl_phys_channel) (This)->lpVtbl->get_PhysicalChannel(This,pl_phys_channel)
#define IATSCLocator_put_PhysicalChannel(This,l_phys_channel) (This)->lpVtbl->put_PhysicalChannel(This,l_phys_channel)
#define IATSCLocator_get_TSID(This,pl_tsid) (This)->lpVtbl->get_TSID(This,pl_tsid)
#define IATSCLocator_put_TSID(This,l_tsid) (This)->lpVtbl->put_TSID(This,l_tsid)

#endif /*COBJMACROS*/


#endif // __BDADEFS_H
