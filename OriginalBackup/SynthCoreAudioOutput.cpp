#include "stdafx.h"
#include "SynthCoreAudioOutput.h"
#include <avrt.h>
#include "SynthEngine.h"

CSynthCoreAudioOutput::CSynthCoreAudioOutput( CSynthParameters *pParams, CSynthObject *pParent,IMMDevice *Endpoint):CSynthObject(OBJECT_TYPE_COREAUDIO_EXCLUSIVE, pParams, pParent)
{
	m_hRenderThread = 0;
	m_hShutdownEvent = 0;
	m_hAudioSamplesReadyEvent = 0;
	m_pEndpoint = Endpoint;
	m_pAudioClient = 0;
	m_pRenderClient = 0;
	m_pMixFormat = 0;
	m_nFrameSize = 0;
	m_nBufferSize = 0;
	m_nBufferSizePerPeriod = 0;
	m_nEngineLatencyInMS = 0;
	m_pRenderBufferQueue = 0;
	m_ppRendBuffQueueTail = &m_pRenderBufferQueue;
}


CSynthCoreAudioOutput::~CSynthCoreAudioOutput()
{
}


bool CSynthCoreAudioOutput::Create(int DevID,int Latency)
{
	//
	//  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
	//
	m_hShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	m_hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

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
		return DisplayError(L"Failed to load the mix format \n",0);

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


bool CSynthCoreAudioOutput::Open()
{
	return Start();
}


void CSynthCoreAudioOutput::Close()
{
	Stop();
}

bool CSynthCoreAudioOutput::Start()
{
	HRESULT hr;

	//------------------------------------------
	// generate initial buffers
	//------------------------------------------
	RenderBuffer *pRendBuffer;
	int i;

	for (i = 0; i < 6; ++i)
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
	m_hRenderThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) CSynthCoreAudioOutput::RenderThread, this, 0, NULL);
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

void CSynthCoreAudioOutput::Stop()
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
	if (m_hRenderThread)
	{
		WaitForSingleObject(m_hRenderThread, INFINITE);

		CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}
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

UINT CSynthCoreAudioOutput::RenderThread(LPVOID param)
{
	CSynthCoreAudioOutput *renderer = (CSynthCoreAudioOutput *)param;
	return renderer->DoRenderThread();
}

UINT CSynthCoreAudioOutput::DoRenderThread()
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
		DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
		switch (waitResult)
		{
		case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
			stillPlaying = false;       // We're done, exit the loop.
			break;
		case WAIT_OBJECT_0 + 1:     // _AudioSamplesReadyEvent
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
				pSE->GenerateSamples(renderBuffer->m_pBuffer, pSE->GetParams()->GetSamplesPerBlock(),ChannelCount() , SampleType());
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

bool CSynthCoreAudioOutput::StartThread()
{
	return false;
}


bool CSynthCoreAudioOutput::TerminateThread()
{
	return false;
}


bool CSynthCoreAudioOutput::DisplayError(LPCWSTR Message, int Error)
{
	CString s;
	s.Format(L"ERROR:%s  Value=%08x", Message, Error);
	MessageBoxW(NULL, s, L"Error", MB_OK | MB_ICONHAND);
	return false;
}

bool CSynthCoreAudioOutput::LoadFormat()
{
	HRESULT hr = m_pAudioClient->GetMixFormat(&m_pMixFormat);
	if (FAILED(hr))
		return DisplayError(L"Unable to get mix format on audio client",hr);
	assert(m_pMixFormat != NULL);

	hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, m_pMixFormat, NULL);
	if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT)
	{
		DisplayError(L"Device does not natively\nsupport the mix format,\n converting to PCM.\n",0);
		//----------------------------------------------------------------------------
		//  If the mix format is a float format, just try to convert the format to PCM.
		//----------------------------------------------------------------------------
		if (m_pMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			m_pMixFormat->wFormatTag = WAVE_FORMAT_PCM;
			m_pMixFormat->wBitsPerSample = 16;
			m_pMixFormat->nBlockAlign = (m_pMixFormat->wBitsPerSample / 8) * m_pMixFormat->nChannels;
			m_pMixFormat->nAvgBytesPerSec = m_pMixFormat->nSamplesPerSec*m_pMixFormat->nBlockAlign;
		}
		else if (m_pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
			reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
		{
			WAVEFORMATEXTENSIBLE *waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat);
			waveFormatExtensible->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			waveFormatExtensible->Format.wBitsPerSample = 16;
			waveFormatExtensible->Format.nBlockAlign = (m_pMixFormat->wBitsPerSample / 8) * m_pMixFormat->nChannels;
			waveFormatExtensible->Format.nAvgBytesPerSec = waveFormatExtensible->Format.nSamplesPerSec*waveFormatExtensible->Format.nBlockAlign;
			waveFormatExtensible->Samples.wValidBitsPerSample = 16;
		}
		else
			return DisplayError(L"Mix format is not a floating point format.\n",0);
		hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, m_pMixFormat, NULL);
		if (FAILED(hr))
			return DisplayError(L"Format is not supported \n",hr);
	}
	m_nFrameSize = m_pMixFormat->nBlockAlign;
	return CalculateMixFormatType();
}


bool CSynthCoreAudioOutput::CalculateMixFormatType()
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
		return DisplayError(L"unrecognized device format.\n",0);
	return true;
}


bool CSynthCoreAudioOutput::InitializeAudioEngine()
{
	REFERENCE_TIME bufferDuration = m_nEngineLatencyInMS * 10000;
	
//	m_pMixFormat->nSamplesPerSec = 48000;
//	m_pMixFormat->nAvgBytesPerSec = m_pMixFormat->nSamplesPerSec * m_pMixFormat->nBlockAlign;

	HRESULT hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		bufferDuration,
		bufferDuration,
		m_pMixFormat,
		NULL);
/*	HRESULT hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		bufferDuration,
		bufferDuration,
		m_pMixFormat,
		NULL); */
	//--------------------------------------------------
	//  When rendering in exclusive mode event driven, 
	//	the HDAudio specification requires that the 
	//	buffers handed to the device must 
	//  be aligned on a 128 byte boundary.  When the
	//	buffer is initialized and the resulting buffer
	//	size would not be 128 byte aligned,
	//  we need to "swizzle" the periodicity of the
	//	engine to ensure that the buffers are properly
	//	aligned.
	//---------------------------------------------------
	if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
	{
		UINT32 bufferSize;
		DisplayError(L"Buffers not aligned. Aligning the buffers... \n", 0);
		//
		//  Retrieve the buffer size for the audio client.  The buffer size returned is aligned to the nearest 128 byte
		//  boundary given the input buffer duration.
		//
		hr = m_pAudioClient->GetBufferSize(&bufferSize);
		if (FAILED(hr))
			return DisplayError(L"Unable to get audio client buffer", hr);
		DisplayInfo(L"Get Buffer Size = ", bufferSize);	//for debug
		//----------------------------------
		//  Release old AudioClient
		//----------------------------------
		if (m_pAudioClient)
		{
			m_pAudioClient->Release();
			m_pAudioClient = 0;
		}
		//
		//  Calculate the new aligned periodicity.  We do that by taking the buffer size returned (which is in frames),
		//  multiplying it by the frames/second in the render format (which gets us seconds per buffer), then converting the 
		//  seconds/buffer calculation into a REFERENCE_TIME.
		//
		bufferDuration = (REFERENCE_TIME)(
			10000.0 *                         // (REFERENCE_TIME / ms) *
			1000 *                            // (ms / s) *
			bufferSize /                      // frames /
			m_pMixFormat->nSamplesPerSec +    // (frames / s)
			0.5);                             // rounding

		//
		//  Now reactivate an IAudioClient object on our preferred endpoint and reinitialize AudioClient
		//
		hr = m_pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_pAudioClient));
		if (FAILED(hr))
			return DisplayError(L"Unable to activate audio client", hr);
		DisplayInfo(L"Buffer Duration = ", bufferDuration);
		hr = m_pAudioClient->Initialize(
			AUDCLNT_SHAREMODE_EXCLUSIVE,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
			bufferDuration,
			bufferDuration,
			m_pMixFormat,
			NULL);
		if (FAILED(hr))
			return DisplayError(L"Unable to reinitialize audio client:", hr);
	}
	else if (FAILED(hr))
		return DisplayError(L"Unable to initialize audio client:", hr);
	//--------------------------------------------------
	//  Retrieve the buffer size for the audio client.
	//--------------------------------------------------
	hr = m_pAudioClient->GetBufferSize(&m_nBufferSize);
	if (FAILED(hr))
		return DisplayError(L"Unable to get audio client buffer:", hr);
	DisplayInfo(L"Buffer Size\n", m_nBufferSize);
	hr = m_pAudioClient->SetEventHandle(m_hAudioSamplesReadyEvent);
	if (FAILED(hr))
		return DisplayError(L"Unable to get set event handle:",hr);
	//
	//  When rendering in event driven mode, we'll always have exactly a buffer's size worth of data
	//  available every time we wake up.
	//
	m_nBufferSizePerPeriod = m_nBufferSize;
	DisplayInfo(L"Buffer Size Per Period =", m_nBufferSizePerPeriod);
	hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pRenderClient));
	if (FAILED(hr))
		return DisplayError(L"Unable to get new render client:", hr);
	return true;
}


bool CSynthCoreAudioOutput::DisplayInfo(LPCWSTR Message, int Value)
{
	CString s;
	s.Format(L"INFO:%s  =%08x", Message, Value);
	MessageBoxW(NULL, s, L"Error", MB_OK | MB_ICONINFORMATION);
	return true;
}

void CSynthCoreAudioOutput::Run()
{
	if (m_pIN)
	{
		float d = m_pIN->GetData();
		if (d > 1.0) d = 1.0;
		else if (d < -1.0) d = -1.0;
		m_Data = d;
	}
}


void CSynthCoreAudioOutput::RunSynthEngine()
{
}
