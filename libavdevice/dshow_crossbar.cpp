extern "C" {
  #include "avdevice.h"
  #include <dshow.h>
}

static const char * GetPhysicalPinName(long lType)
{
    switch (lType)
    {
    case PhysConn_Video_Tuner:            return "Video Tuner";
    case PhysConn_Video_Composite:        return "Video Composite";
    case PhysConn_Video_SVideo:           return "S-Video";
    case PhysConn_Video_RGB:              return "Video RGB";
    case PhysConn_Video_YRYBY:            return "Video YRYBY";
    case PhysConn_Video_SerialDigital:    return "Video Serial Digital";
    case PhysConn_Video_ParallelDigital:  return "Video Parallel Digital";
    case PhysConn_Video_SCSI:             return "Video SCSI";
    case PhysConn_Video_AUX:              return "Video AUX";
    case PhysConn_Video_1394:             return "Video 1394";
    case PhysConn_Video_USB:              return "Video USB";
    case PhysConn_Video_VideoDecoder:     return "Video Decoder";
    case PhysConn_Video_VideoEncoder:     return "Video Encoder";

    case PhysConn_Audio_Tuner:            return "Audio Tuner";
    case PhysConn_Audio_Line:             return "Audio Line";
    case PhysConn_Audio_Mic:              return "Audio Microphone";
    case PhysConn_Audio_AESDigital:       return "Audio AES/EBU Digital";
    case PhysConn_Audio_SPDIFDigital:     return "Audio S/PDIF";
    case PhysConn_Audio_SCSI:             return "Audio SCSI";
    case PhysConn_Audio_AUX:              return "Audio AUX";
    case PhysConn_Audio_1394:             return "Audio 1394";
    case PhysConn_Audio_USB:              return "Audio USB";
    case PhysConn_Audio_AudioDecoder:     return "Audio Decoder";

    default:                              return "Unknown Type";
    }
}


HRESULT SetupCrossbarOptions(IAMCrossbar *pXBar, int video_input_pin, int audio_input_pin, const char *device_name)
{
    HRESULT hr;
    long cOutput = -1, cInput = -1;
    hr = pXBar->get_PinCounts(&cOutput, &cInput);
    int i;
    printf("CrossBar Information for %s:\n", device_name);
    for (i = 0; i < cOutput; i++)
    {
        long lRelated = -1, lType = -1, lRouted = -1;

        hr = pXBar->get_CrossbarPinInfo(FALSE, i, &lRelated, &lType);
        if (lType == PhysConn_Video_VideoDecoder) {
			// assume there is only one "Video (and one Audio) Decoder" output pin, and it's all we care about routing to...for now
			if (video_input_pin != -1) {
				av_log(NULL, AV_LOG_DEBUG, "routing video input from pin %d\n", video_input_pin);
				if(pXBar->Route(i, video_input_pin) != S_OK) {
				  av_log(NULL, AV_LOG_ERROR, "unable to route video input from pin %d\n", video_input_pin);
				  return -1;
				}
			}
		} else if (lType == PhysConn_Audio_AudioDecoder) {
        	if (audio_input_pin != -1) {
				av_log(NULL, AV_LOG_DEBUG, "routing audio input from pin %d\n", audio_input_pin);
				if(pXBar->Route(i, audio_input_pin) != S_OK) {
				    av_log(NULL, AV_LOG_ERROR, "unable to route audio input from pin %d\n", audio_input_pin);
				    return -1;
				}
			}
		} else {
			av_log(NULL, AV_LOG_WARNING, "unexpected output pin type, please report the type if you want to use this (%s)", GetPhysicalPinName(lType));
		}

		hr = pXBar->get_IsRoutedTo(i, &lRouted);

        printf("Output pin %d: %s\n", i, GetPhysicalPinName(lType)); // like "Video Decoder"
		
        printf("\tRelated out: %ld, Currently Routed in: %ld\n", lRelated, lRouted);
        printf("\tSwitching Compatibility: ");

        for (int j = 0; j < cInput; j++)
        {
            hr = pXBar->CanRoute(i, j);
            printf("%d-%s ", j, (S_OK == hr ? "Yes" : "No"));
        }
        printf("\n");
    }

    for (i = 0; i < cInput; i++)
    {
        long lRelated = -1, lType = -1;

        hr = pXBar->get_CrossbarPinInfo(TRUE, i, &lRelated, &lType);

        printf("Input pin %d - %s\n", i, GetPhysicalPinName(lType));
        printf("\tRelated in: %ld\n", lRelated);
    }
	return S_OK;
}

extern "C" {

void show_properties(IBaseFilter *pFilter) {
/* Obtain the filter's IBaseFilter interface. (Not shown) */
ISpecifyPropertyPages *pProp;
HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
if (SUCCEEDED(hr)) 
{
    // Get the filter's name and IUnknown pointer.
    FILTER_INFO FilterInfo;
    hr = pFilter->QueryFilterInfo(&FilterInfo); 
    IUnknown *pFilterUnk;
    pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

    // Show the page. 
    CAUUID caGUID;
    pProp->GetPages(&caGUID);
    pProp->Release();
    OleCreatePropertyFrame(
        NULL,                   // Parent window
        0, 0,                   // Reserved
        FilterInfo.achName,     // Caption for the dialog box
        1,                      // Number of objects (just the filter)
        &pFilterUnk,            // Array of object pointers. 
        caGUID.cElems,          // Number of property pages
        caGUID.pElems,          // Array of property page CLSIDs
        0,                      // Locale identifier
        0, NULL                 // Reserved
    );

    // Clean up.
    pFilterUnk->Release();
    if (FilterInfo.pGraph)
      FilterInfo.pGraph->Release(); 
    CoTaskMemFree(caGUID.pElems);
}
}

    HRESULT setup_crossbar_options(ICaptureGraphBuilder2 *graph_builder2, IBaseFilter *device_filter, 
        int crossbar_video_input_pin_number, int crossbar_audio_input_pin_number, const char *device_name) {
        IAMCrossbar *pCrossBar = NULL;
        HRESULT r;
        r = graph_builder2->FindInterface(&LOOK_UPSTREAM_ONLY, (const GUID *) NULL,
                (IBaseFilter *) device_filter, IID_IAMCrossbar, (void**) &pCrossBar);
        if (r == S_OK) {
            /* It found a cross bar device was inserted, optionally configure it */
            r = SetupCrossbarOptions(pCrossBar, crossbar_video_input_pin_number, 
                crossbar_audio_input_pin_number, device_name);
            pCrossBar->Release();
        } else {
            /* no crossbar to setup */
            r = S_OK;
        }
        return r;
    }
}
