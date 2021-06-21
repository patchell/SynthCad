#include "stdafx.h"
#include "SynthAudioSharedOut.h"
#include <avrt.h>
#include "SynthEngine.h"

UINT CSynthAudioSharedOut::RenderThread(LPVOID param)
{
	CSynthAudioSharedOut *pSASO = (CSynthAudioSharedOut *)param;
	return pSASO->DoRenderThread();
}

UINT CSynthAudioSharedOut::DoRenderThread()
{
	bool stillPlaying = true;
	HANDLE waitArray[2] = { m_hShutdownEvent, m_hAudioSamplesReadyEvent };
	HANDLE mmcssHandle = NULL;
	DWORD mmcssTaskIndex = 0;
	int count = 0;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		DisplayError(L"Unable to initialize COM in render thread:", hr);
		return hr;
	}
	mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
	if (mmcssHandle == NULL)
		DisplayInfo(L"Unable to enable MMCSS on render thread:", GetLastError());
	while (stillPlaying)
	{
		DWORD waitResult = WaitForMultipleObjects(3, waitArray, FALSE, INFINITE);
		switch (waitResult)
		{
		case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
			stillPlaying = false;       // We're done, exit the loop.
			break;
		case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
			//
			//  We've received a stream switch request.
			//
			//  We need to stop the renderer, tear down the _AudioClient and _RenderClient objects and re-create them on the new.
			//  endpoint if possible.  If this fails, abort the thread.
			//
			if (!HandleStreamSwitchEvent())
			{
				stillPlaying = false;
			}
			break;
		case WAIT_OBJECT_0 + 2:     // _AudioSamplesReadyEvent
									//
									//  We need to provide the next buffer of samples to the audio renderer.
									//
			BYTE *pData;
			++count;
			//
			//  When rendering in event driven mode, every time we wake up, we'll have a buffer's worth of data available, so if we have
			//  data in our queue, render it.
			//
			//-------------------------------
			// check to see if we are at the
			// end of the queue
			//------------------------------
			if (m_pRenderBufferQueue == NULL)
			{
				//yes, all done
				stillPlaying = false;
			}
			else
			{
				//remove cycle from queue
				RenderBuffer *renderBuffer = m_pRenderBufferQueue;
				m_pRenderBufferQueue = renderBuffer->m_pNext;
				renderBuffer->m_pNext = 0;

				UINT32 framesToWrite = renderBuffer->m_nBufferLength / m_nFrameSize;
				hr = m_pRenderClient->GetBuffer(framesToWrite, &pData);
				if (SUCCEEDED(hr))
				{
					//---------------------------------------
					//  Copy data from the render buffer to 
					//	the output buffer and bump our render
					//	pointer.
					//----------------------------------------
					CopyMemory(pData, renderBuffer->m_pBuffer, framesToWrite*m_nFrameSize);
					hr = m_pRenderClient->ReleaseBuffer(framesToWrite, 0);
					if (!SUCCEEDED(hr))
					{
						DisplayError(L"Unable to release buffer:", hr);
						stillPlaying = false;
					}
				}
				else
				{
					DisplayError(L"Unable to get buffer:", hr);
					stillPlaying = false;
				}
				//--------------------------------------------
				// Generate next set of samples
				//-------------------------------------------
				CSynthEngine *pSE = (CSynthEngine *)GetParent();
				pSE->GenerateSamples(renderBuffer->m_pBuffer, pSE->GetParams()->GetSamplesPerBlock(), ChannelCount(), SampleType());
				//--------------------------------------------
				// add buffer back into queue
				//--------------------------------------------
				*m_ppRendBuffQueueTail = renderBuffer;
				m_ppRendBuffQueueTail = &renderBuffer->m_pNext;
			}
			break;
		}
	}
	AvRevertMmThreadCharacteristics(mmcssHandle);
	CoUninitialize();
	return 0;
}

CSynthAudioSharedOut::CSynthAudioSharedOut(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_COREAUDIO_SHARED,pParams,pParent)
{
}


CSynthAudioSharedOut::~CSynthAudioSharedOut()
{
}

bool CSynthAudioSharedOut::Create(int DevID, int Latency)
{
	m_hShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	m_hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	//
	//  Create our stream switch event- we want auto reset events that start in the not-signaled state.
	//  Note that we create this event even if we're not going to stream switch - that's because the event is used
	//  in the main loop of the renderer and thus it has to be set.
	//
	m_hStreamSwitchEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	//
	//  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
	//
	HRESULT hr = m_pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_pAudioClient));
	if (FAILED(hr))
		return DisplayError(L"Unable to activate audio client", hr);
	//
	// Load the MixFormat.  This may differ depending on the shared mode used
	//
	if (!LoadFormat())
		return DisplayError(L"Failed to load the mix format \n", 0);

	//
	//  Remember our configured latency in case we'll need it for a stream switch later.
	//
	m_nEngineLatencyInMS = Latency;
	if (!InitializeAudioEngine())
	{
		return false;
	}

	return true;
}

bool CSynthAudioSharedOut::Open()
{
	return Start();
}
void CSynthAudioSharedOut::Close()
{
	Stop();
}
bool CSynthAudioSharedOut::Start()
{
	HRESULT hr;

	//------------------------------------------
	// generate initial buffers
	//------------------------------------------
	RenderBuffer *pRendBuffer;
	int i;

	for (i = 0; i < 3; ++i)
	{
		pRendBuffer = new RenderBuffer;
		pRendBuffer->m_pBuffer = new BYTE[FrameSize() * BufferSize()];
		pRendBuffer->m_nBufferLength = BufferSize() * FrameSize();
		//--------------------------------------------
		// Generate next set of samples
		//-------------------------------------------
		CSynthEngine *pSE = (CSynthEngine *)GetParent();
		pSE->GenerateSamples(pRendBuffer->m_pBuffer, BufferSize(), ChannelCount(), SampleType());
		//--------------------------------------------
		// add buffer into queue
		//--------------------------------------------
		*m_ppRendBuffQueueTail = pRendBuffer;
		m_ppRendBuffQueueTail = &pRendBuffer->m_pNext;
	}

	//------------------------------------------
	//  Now create the thread which is going to
	//	drive the renderer.
	//-------------------------------------------
	m_hRenderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSynthCoreAudioOutput::RenderThread, this, 0, NULL);
	if (m_hRenderThread == NULL)
		return DisplayError(L"Unable to create transport thread:", GetLastError());
	//-----------------------------------------
	//  We want to pre-roll the first buffer's
	//	worth of data into the pipeline.
	//	That way the audio engine won't glitch
	//	on startup.  
	//-----------------------------------------
	{
		BYTE *pData;

		if (m_pRenderBufferQueue != NULL)
		{
			//----------------------------------------
			//  Remove the buffer from the queue.
			//----------------------------------------
			RenderBuffer *renderBuffer = m_pRenderBufferQueue;
			m_pRenderBufferQueue = renderBuffer->m_pNext;
			DWORD bufferLengthInFrames = renderBuffer->m_nBufferLength / m_nFrameSize;

			hr = m_pRenderClient->GetBuffer(bufferLengthInFrames, &pData);
			if (FAILED(hr))
				return DisplayError(L"Failed to get buffer:", hr);
			CopyMemory(pData, renderBuffer->m_pBuffer, renderBuffer->m_nBufferLength);
			hr = m_pRenderClient->ReleaseBuffer(bufferLengthInFrames, 0);

			delete renderBuffer;
		}
		else
		{
			hr = m_pRenderClient->GetBuffer(m_nBufferSize, &pData);
			if (FAILED(hr))
				return DisplayError(L"Failed to get buffer:", hr);
			hr = m_pRenderClient->ReleaseBuffer(m_nBufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
		}
		if (FAILED(hr))
			return DisplayError(L"Failed to release buffer:", hr);
	}
	//----------------------------------------
	//  We're ready to go, start rendering!
	//----------------------------------------
	hr = m_pAudioClient->Start();
	if (FAILED(hr))
		return DisplayError(L"Unable to start render client::", hr);
	return true;

}
void CSynthAudioSharedOut::Stop()
{
	HRESULT hr;

	//
	//  Tell the render thread to shut down, wait for the thread to complete then clean up all the stuff we 
	//  allocated in Start().
	//
	if (m_hShutdownEvent)
	{
		SetEvent(m_hShutdownEvent);
	}

	hr = m_pAudioClient->Stop();
	if (FAILED(hr))
		DisplayError(L"Unable to stop audio client:", hr);
	TerminateThread();
	//-------------------------------------------------
	//  Drain the buffers in the render buffer queue.
	//--------------------------------------------------
	while (m_pRenderBufferQueue != NULL)
	{
		RenderBuffer *renderBuffer = m_pRenderBufferQueue;
		m_pRenderBufferQueue = renderBuffer->m_pNext;
		delete renderBuffer;
	}
}
bool CSynthAudioSharedOut::StartThread()
{
	return false;
}

bool CSynthAudioSharedOut::TerminateThread()
{
	if (m_hRenderThread)
	{
		WaitForSingleObject(m_hRenderThread, INFINITE);

		CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}
	return true;
}


bool CSynthAudioSharedOut::CalculateMixFormatType()
{
	if (m_pMixFormat->wFormatTag == WAVE_FORMAT_PCM ||
		m_pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
		reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
	{
		if (m_pMixFormat->wBitsPerSample == 16)
		{
			m_eRenderSampleType = SampleType16BitPCM;
		}
		else
			return DisplayError(L"Unknown PCM integer sample type\n", 0);
	}
	else if (m_pMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
		(m_pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
			reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
	{
		m_eRenderSampleType = SampleTypeFloat;
	}
	else
		return DisplayError(L"unrecognized device format.\n", 0);
	return true;
}

bool CSynthAudioSharedOut::LoadFormat()
{
	HRESULT hr = m_pAudioClient->GetMixFormat(&m_pMixFormat);
	if (FAILED(hr))
		return DisplayError(L"Unable to get mix format on audio client", hr);
	assert(m_pMixFormat != NULL);
	m_nFrameSize = m_pMixFormat->nBlockAlign;
	return CalculateMixFormatType();
}

bool CSynthAudioSharedOut::InitializeAudioEngine()
{
	REFERENCE_TIME bufferDuration = m_nEngineLatencyInMS * 10000;

	//	m_pMixFormat->nSamplesPerSec = 48000;
	//	m_pMixFormat->nAvgBytesPerSec = m_pMixFormat->nSamplesPerSec * m_pMixFormat->nBlockAlign;

	HRESULT hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
			bufferDuration,
			0,
			m_pMixFormat,
			NULL);
	hr = m_pAudioClient->GetBufferSize(&m_nBufferSize);
	if (FAILED(hr))
		return DisplayError(L"Unable to get audio client buffer:", hr);
	DisplayInfo(L"Buffer Size\n", m_nBufferSize);
	hr = m_pAudioClient->SetEventHandle(m_hAudioSamplesReadyEvent);
	if (FAILED(hr))
		return DisplayError(L"Unable to get set event handle:", hr);
	m_nBufferSizePerPeriod = m_nBufferSize;
	DisplayInfo(L"Buffer Size Per Period =", m_nBufferSizePerPeriod);
	hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pRenderClient));
	if (FAILED(hr))
		return DisplayError(L"Unable to get new render client:", hr);
	return true;
}


bool CSynthAudioSharedOut::DisplayInfo(LPCWSTR Message, int Value)
{
	CString s;
	s.Format(L"INFO:%s  =%08x", Message, Value);
	MessageBoxW(NULL, s, L"Error", MB_OK | MB_ICONINFORMATION);
	return true;
}

bool CSynthAudioSharedOut::DisplayError(LPCWSTR Message, int Error)
{
	//one
	CString s;
	s.Format(L"ERROR:%s  Value=%08x", Message, Error);
	MessageBoxW(NULL, s, L"Error", MB_OK | MB_ICONHAND);
	return false;
}


//
//  Handle the stream switch.
//
//  When a stream switch happens, we want to do several things in turn:
//
//  1) Stop the current renderer.
//  2) Release any resources we have allocated (the _AudioClient, _AudioSessionControl (after unregistering for notifications) and 
//        _RenderClient).
//  3) Wait until the default device has changed (or 500ms has elapsed).  If we time out, we need to abort because the stream switch can't happen.
//  4) Retrieve the new default endpoint for our role.
//  5) Re-instantiate the audio client on that new endpoint.  
//  6) Retrieve the mix format for the new endpoint.  If the mix format doesn't match the old endpoint's mix format, we need to abort because the stream
//      switch can't happen.
//  7) Re-initialize the _AudioClient.
//  8) Re-register for session disconnect notifications and reset the stream switch complete event.
//
bool CSynthAudioSharedOut::HandleStreamSwitchEvent()
{
	HRESULT hr;

	ASSERT(m_bInStreamSwitch);
	//
	//  Step 1.  Stop rendering.
	//
	hr = m_pAudioClient->Stop();
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to stop audio client\n during stream switch:", hr);
	//
	//  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
	//
	hr = m_pAudioSessionControl->UnregisterAudioSessionNotification(this);
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to stop audio client during stream switch", hr);

	delete m_pAudioSessionControl;
	m_pAudioSessionControl = 0;

	delete m_pRenderClient;
	m_pRenderClient = 0;

	delete m_pAudioClient;
	m_pAudioClient = 0;

	delete m_pEndpoint;
	m_pEndpoint = 0;

	//
	//  Step 3.  Wait for the default device to change.
	//
	//  There is a race between the session disconnect arriving and the new default device 
	//  arriving (if applicable).  Wait the shorter of 500 milliseconds or the arrival of the 
	//  new default device, then attempt to switch to the default device.  In the case of a 
	//  format change (i.e. the default device does not change), we artificially generate  a
	//  new default device notification so the code will not needlessly wait 500ms before 
	//  re-opening on the new format.  (However, note below in step 6 that in this SDK 
	//  sample, we are unlikely to actually successfully absorb a format change, but a 
	//  real audio application implementing stream switching would re-format their 
	//  pipeline to deliver the new format).  
	//
	DWORD waitResult = WaitForSingleObject(m_hStreamSwitchCompleteEvent, 500);
	if (waitResult == WAIT_TIMEOUT)
		return m_bInStreamSwitch = DisplayError(L"Stream switch timeout - aborting...", 0);

	//
	//  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
	//          we should be able to retrieve it.
	//
	hr = m_pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, m_eEndpointRole, &m_pEndpoint);
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to retrieve new default device during stream switch:", hr);
	//
	//  Step 5 - Re-instantiate the audio client on the new endpoint.
	//
	hr = m_pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_pAudioClient));
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to activate audio client on the new endpoint", hr);
	//
	//  Step 6 - Retrieve the new mix format.
	//
	WAVEFORMATEX *wfxNew;
	hr = m_pAudioClient->GetMixFormat(&wfxNew);
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to retrieve mix format for new audio client:", hr);
	//
	//  Note that this is an intentionally naive comparison.  A more sophisticated comparison would
	//  compare the sample rate, channel count and format and apply the appropriate conversions into the render pipeline.
	//
	if (memcmp(m_pMixFormat, wfxNew, sizeof(WAVEFORMATEX) + wfxNew->cbSize) != 0)
	{
		DisplayInfo(L"New mix format doesn't match old mix format.  Aborting.\n",0);
		CoTaskMemFree(wfxNew);
		goto ErrorExit;
	}
	CoTaskMemFree(wfxNew);

	//
	//  Step 7:  Re-initialize the audio client.
	//
	if (!InitializeAudioEngine())
	{
		goto ErrorExit;
	}

	//
	//  Step 8: Re-register for session disconnect notifications.
	//
	hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pAudioSessionControl));
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to retrieve session control on new audio client:", hr);
	hr = m_pAudioSessionControl->RegisterAudioSessionNotification(this);
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to retrieve session control on new audio client:", hr);
	//
	//  Reset the stream switch complete event because it's a manual reset event.
	//
	ResetEvent(m_hStreamSwitchCompleteEvent);
	//
	//  And we're done.  Start rendering again.
	//
	hr = m_pAudioClient->Start();
	if (FAILED(hr))
		return m_bInStreamSwitch = DisplayError(L"Unable to start the new audio client::", hr);
	m_bInStreamSwitch = false;
	return true;

ErrorExit:
	m_bInStreamSwitch = false;
	return false;
}


bool CSynthAudioSharedOut::InitializeStreamSwitch()
{
	HRESULT hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pAudioSessionControl));
	if (FAILED(hr))
		return DisplayError(L"Unable to retrieve session control", hr);
	//
	//  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
	//
	m_hStreamSwitchCompleteEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
	if (m_hStreamSwitchCompleteEvent == NULL)
		return DisplayError(L"Unable to create stream switch event:", GetLastError());
	//
	//  Register for session and endpoint change notifications.  
	//
	//  A stream switch is initiated when we receive a session disconnect notification or we receive a default device changed notification.
	//
	hr = m_pAudioSessionControl->RegisterAudioSessionNotification(this);
	if (FAILED(hr))
		return DisplayError(L"Unable to register for stream switch notifications", hr);
	hr = m_pDeviceEnumerator->RegisterEndpointNotificationCallback(this);
	if (FAILED(hr))
		return DisplayError(L"Unable to register for stream switch notifications:", hr);
	return true;
}

void CSynthAudioSharedOut::Run()
{
	if (m_pIN)
	{
		float d = m_pIN->GetData();
		if (d > 1.0) d = 1.0;
		else if (d < -1.0) d = -1.0;
		m_Data = d;
	}
}
