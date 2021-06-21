#include "stdafx.h"
#include "SynthCadDefines.h"
#include <mmsystem.h>
#include <mmsyscom.h>
#include "SynthEngine.h"
#include "SynthCoreAudioSharedOut.h"
#include <avrt.h>

bool DisableMMCSS;

CSynthCoreAudioSharedOut::CSynthCoreAudioSharedOut(CSynthParameters *pParams, CSynthObject *pParent, IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole)
	:CSynthObject(OBJECT_TYPE_AUDIO,pParams,pParent)
{
	m_pEndpoint= Endpoint;
	m_pAudioClient = 0;
	m_pRenderClient = 0;
	m_nRefCount = 0;
	m_hRenderThread = 0;
	m_hShutdownEvent = 0;
	m_hAudioSamplesReadyEvent = 0;
	m_pMixFormat = 0;
	m_nFrameSize = 0;
	m_pEndpoint->AddRef();    // Since we're holding a copy of the endpoint, take a reference to it.  It'll be released in Shutdown();

	m_pIN = 0;
	m_pRenderQueue = NULL;
	m_ppRenderQueueTail = &m_pRenderQueue;
}


CSynthCoreAudioSharedOut::~CSynthCoreAudioSharedOut()
{
}

void CSynthCoreAudioSharedOut::Open()
{
	Initialize(30);
//	Start();
}

void CSynthCoreAudioSharedOut::Close()
{
}

void CSynthCoreAudioSharedOut::Run()
{
	if (m_pIN)
	{
		float d = m_pIN->GetData();
		if (d > 1.0) d = 1.0;
		else if (d < -1.0) d = -1.0;
		m_Data = d;
	}
}

void CSynthCoreAudioSharedOut::RunSynthEngine(short * pBuf)
{
	CSynthEngine *pENG = (CSynthEngine *)GetParent();
	int i, n;

	n = GetParams()->GetSamplesPerBlock();
	for (i = 0; i < n; ++i)
	{
		pENG->Run();	//run one sysnth cycle n times
		pBuf[i] = (short)GetData();
	}

}

bool CSynthCoreAudioSharedOut::Create(CPoint p1)
{
	return CSynthObject::Create(p1);
}

//-----------------------------
// Render Methods
//-----------------------------
bool CSynthCoreAudioSharedOut::InitializeAudioEngine()
{
	//-------------------------------------------
	//	InitializeAudioEngine
	//	Return Value:
	//		true....success
	//		false...FAIL!
	//-------------------------------------------
	HRESULT hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		m_nEngineLatencyInMS * 10000,
		0,
		m_pMixFormat,
		NULL);

	if (FAILED(hr))
	{
		CString s;
		s.Format(L"Unable to initialize audio client: %x", hr);
		MessageBox(NULL, s, L"ERROR!!!", MB_OK | MB_ICONHAND);
		return false;
	}
	//-------------------------------------------------
	//  Retrieve the buffer size for the audio client.
	//--------------------------------------------------
	hr = m_pAudioClient->GetBufferSize(&m_nBufferSize);
	if (FAILED(hr)) return false;
	//-------------------------------------------------
	//	Set the event handle that we will use to
	// let us know when the audio client is ready for
	// for more samples.
	//-------------------------------------------------
	hr = m_pAudioClient->SetEventHandle(m_hAudioSamplesReadyEvent);
	if (FAILED(hr)) return false;
	hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pRenderClient));
	if (FAILED(hr))return false;
	return true;	//method completed succesfully
}

//
//  The Event Driven renderer will be woken up every defaultDevicePeriod hundred-nano-seconds.
//  Convert that time into a number of frames.
//
UINT32 CSynthCoreAudioSharedOut::BufferSizePerPeriod()
{
	//-------------------------------------------
	//	BufferSizePerPeriod
	//		What we need to do is to calculate
	// how many samples there will be per
	// period.  To do this, we call 
	// GetDevicePeriod, which will give us the
	// time in 100's of nanoseconds.  We need
	// to convert that into seconds.  We then
	// multiply this number by samplesPerSecond
	// which will give us the number of samples
	// needed per buffer.
	//-------------------------------------------
	REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;
	HRESULT hr = m_pAudioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
	if (FAILED(hr)) return 0;
	double devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
	return static_cast<UINT32>(m_pMixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

bool CSynthCoreAudioSharedOut::LoadFormat()
{
	//------------------------------------------
	//	LoadFormat
	//		This methode is used to get the
	// format of the wav stream from the
	// audio client.  When retrieved, the
	// data memeber m_pMixFormat will point
	// to the data
	//
	// return value:
	// returns true on success, false on fail
	//------------------------------------------
	HRESULT hr = m_pAudioClient->GetMixFormat(&m_pMixFormat);
	if (FAILED(hr)) return false;
	m_nFrameSize = m_pMixFormat->nBlockAlign;
	if (!CalculateMixFormatType()) return false;
	return true;
}

bool CSynthCoreAudioSharedOut::CalculateMixFormatType()
{
	//------------------------------------------------------
	// CalculateMixFormatType
	//	This method is used to determine what
	// sort of data format the steam has.  We
	// need to see if it is floating point or
	// integer format.
	//
	// return value:
	//	true on success, or false on fail
	//------------------------------------------------------
	if (m_pMixFormat->wFormatTag == WAVE_FORMAT_PCM ||
		m_pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
		reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
	{
		if (m_pMixFormat->wBitsPerSample == 16)
		{
			m_eRenderSampleType = SampleType16BitPCM;
		}
		else
		{
			CString s;

			s.Format(L"Unknown PCM integer sample type");
			MessageBox(NULL, s, L"ERROR!", MB_OK | MB_ICONHAND);
			return false;
		}
	}
	else if (m_pMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
		(m_pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
		reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_pMixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
	{
		m_eRenderSampleType = SampleTypeFloat;
	}
	else
	{
		CString s;
		s.Format(L"unrecognized device format");
		MessageBox(NULL, s, L"ERROR!", MB_OK | MB_ICONHAND);
		return false;
	}
	return true;
}
//
//  Initialize the renderer.
//
bool CSynthCoreAudioSharedOut::Initialize(UINT32 EngineLatency)
{
	if (EngineLatency < 30) return false;
	//---------------------------------------
	//  Create our shutdown and samples ready
	//	events- we want auto reset events
	//	that start in the not-signaled state.
	//---------------------------------------
	m_hShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	m_hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	//---------------------------------------
	//  Now activate an IAudioClient object
	//	on our preferred endpoint and 
	//	retrieve the mix format for that 
	//	endpoint.
	//---------------------------------------
	HRESULT hr = m_pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_pAudioClient));
	if (FAILED(hr)) return false;
	//------------------------------------
	// Load the MixFormat.  This may 
	// differ depending on the shared 
	// mode used
	//------------------------------------
	if (!LoadFormat()) return false;
	wprintf(L"Sample Rate = %d\n", m_pMixFormat->nSamplesPerSec);
	//-------------------------------------
	//  Remember our configured latency in
	//	case we'll need it for a 
	//	stream switch later, which we won't
	// because we don't handle that.
	//-------------------------------------
	m_nEngineLatencyInMS = EngineLatency;

	return InitializeAudioEngine();
}

//
//  Shut down the render code and free all the resources.
//
void CSynthCoreAudioSharedOut::Shutdown()
{
	if (m_hRenderThread)
	{
		SetEvent(m_hShutdownEvent);
		WaitForSingleObject(m_hRenderThread, INFINITE);
		CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}

	if (m_hShutdownEvent)
	{
		CloseHandle(m_hShutdownEvent);
		m_hShutdownEvent = NULL;
	}
	if (m_hAudioSamplesReadyEvent)
	{
		CloseHandle(m_hAudioSamplesReadyEvent);
		m_hAudioSamplesReadyEvent = NULL;
	}

	SafeRelease(&m_pEndpoint);
	SafeRelease(&m_pAudioClient);
	SafeRelease(&m_pRenderClient);

	if (m_pMixFormat)
	{
		CoTaskMemFree(m_pMixFormat);
		m_pMixFormat = NULL;
	}
}


//
//  Start rendering - Create the render thread and start rendering the buffer.
//
bool CSynthCoreAudioSharedOut::Start(RenderBuffer *RenderBufferQueue)
{
	//----------------------------------------------
	//	Start
	//		Once the audio engine has been initialized
	// we call this method to start everything going.
	//
	//	parameters:
	//		RenderBufferQueue.....pointer to the queue
	//							that contains the
	//							initial batch of samples.
	//
	// return Value:
	//		true......success
	//		false.....fail
	//--------------------------------------------------
	HRESULT hr;

	//------------------------------------------
	// We will take the first buffer and send
	// it to the engine.  After we are done
	// with it, it will get thrown away.  So
	// one should note that when creating
	// the buffer queue, there should be a
	// minimum of 3 buffers in it.
	//------------------------------------------
	{
		BYTE *pData;

		if (m_pSE->m_pAudioOut->m_pRenderQueue != NULL)
		{
			//-------------------------------------
			//  Remove the buffer from the queue.
			//-------------------------------------
			RenderBuffer *renderBuffer = m_pSE->m_pAudioOut->m_pRenderQueue;
			m_pSE->m_pAudioOut->m_pRenderQueue = renderBuffer->m_pNext;
			DWORD bufferLengthInFrames = renderBuffer->m_nBufferLength / m_nFrameSize;

			hr = m_pRenderClient->GetBuffer(bufferLengthInFrames, &pData);
			if (FAILED(hr)) return false;
			CopyMemory(pData, renderBuffer->m_pBuffer, renderBuffer->m_nBufferLength);
			hr = m_pRenderClient->ReleaseBuffer(bufferLengthInFrames, 0);
			//throw away the one buffer
			//because to recover it will
			// be just plain messy.
			delete renderBuffer;
		}
		else
		{
			hr = m_pRenderClient->GetBuffer(m_nBufferSize, &pData);
			if (FAILED(hr)) return false;
			hr = m_pRenderClient->ReleaseBuffer(m_nBufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
		}
		if (FAILED(hr)) 
			return false;	//failed to release buffer
	}

	//---------------------------------
	//  Now create the thread which is
	//	going to drive the renderer.
	//---------------------------------
	m_hRenderThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) WASAPIRenderThread, this, 0, NULL);
	if (m_hRenderThread == NULL) return false;
	//---------------------------------------
	//  We're ready to go, start rendering!
	//---------------------------------------
	hr = m_pAudioClient->Start();
	if (FAILED(hr)) return false;
	return true;
}

//
//  Stop the renderer.
//
void CSynthCoreAudioSharedOut::Stop()
{
	HRESULT hr;

	//-------------------------------------
	//  Tell the Render Thread to shut down
	//-------------------------------------
	if (m_hShutdownEvent) SetEvent(m_hShutdownEvent);
	hr = m_pAudioClient->Stop();
	if (FAILED(hr))
	{
		printf("Unable to stop audio client: %x\n", hr);
	}
	//-------------------------------------
	// wait for the thread to quit
	//------------------------------------
	if (m_hRenderThread)
	{
		WaitForSingleObject(m_hRenderThread, INFINITE);
		CloseHandle(m_hRenderThread);
		m_hRenderThread = NULL;
	}

	//-----------------------------------------------
	//  Drain the buffers in the render buffer queue.
	//-----------------------------------------------
	while (m_pSE->m_pAudioOut->m_pRenderQueue != NULL)
	{
		RenderBuffer *renderBuffer = m_pSE->m_pAudioOut->m_pRenderQueue;
		m_pSE->m_pAudioOut->m_pRenderQueue = renderBuffer->m_pNext;
		delete renderBuffer;
	}

}


DWORD CSynthCoreAudioSharedOut::WASAPIRenderThread(LPVOID Context)
{
	//-------------------------------------------
	// WASAPIRenderThread
	//	This is where the samples are generated
	// and played through the audio port
	//
	//	parameters:
	//		Context......value that we set to
	//					point to the
	//					CSynthCoreAudioSharedOut
	//					object (i.e. this)
	//	return value:
	//		we just return what DoRenderThread
	//	returns.
	//-------------------------------------------
	CSynthCoreAudioSharedOut *renderer = static_cast<CSynthCoreAudioSharedOut *>(Context);
	return renderer->DoRenderThread();
}

DWORD CSynthCoreAudioSharedOut::DoRenderThread()
{
	bool stillPlaying = true;
	HANDLE waitArray[2] = { m_hShutdownEvent, m_hAudioSamplesReadyEvent };
	HANDLE mmcssHandle = NULL;
	DWORD mmcssTaskIndex = 0;

	//---------------------------------------
	// Initialize COM
	//----------------------------------------
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) return hr;
	if (!DisableMMCSS)
	{
		mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
	}
	//---------------------------------------------
	// loop here until told to stop
	//---------------------------------------------
	while (stillPlaying)
	{
		DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
		switch (waitResult)
		{
		case WAIT_OBJECT_0 + 0:     // m_hShutdownEvent
			stillPlaying = false;       // We're done, exit the loop.
			break;
		case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
			//--------------------------------------
			//  We need to provide the next buffer
			//	of samples to the audio renderer.
			//--------------------------------------
			BYTE *pData;
			UINT32 padding;
			UINT32 framesAvailable;
			
			//-----------------------------------
			//  We want to find out how much of
			//	the buffer *isn't* available
			//	(is padding).
			//-----------------------------------
			hr = m_pAudioClient->GetCurrentPadding(&padding);
			if (SUCCEEDED(hr))
			{
				//---------------------------------
				//  Calculate the number of frames
				//	available.  We'll render
				//  that many frames or the number
				//	of frames left in the buffer,
				//	whichever is smaller.
				//--------------------------------
				framesAvailable = m_nBufferSize - padding;

				//---------------------------------
				//  If the buffer at the head of
				//	the render buffer queue fits
				//	in the frames available, 
				//	render it.  If we don't
				//  have enough room to fit the
				//	buffer, skip this pass - we 
				//	will have enough room on the
				//	next pass.
				//--------------------------------
				if (m_pSE->m_pAudioOut->m_pRenderQueue->m_nBufferLength <= (framesAvailable *m_nFrameSize))
				{
					//-----------------------------------
					// remove buffer
					//-----------------------------------

					RenderBuffer *renderBuffer = m_pSE->m_pAudioOut->m_pRenderQueue;
					m_pSE->m_pAudioOut->m_pRenderQueue = renderBuffer->m_pNext;
					renderBuffer->m_pNext = 0;
					//----------------------------------------
					// copy data out of it
					//----------------------------------------
					UINT32 framesToWrite = renderBuffer->m_nBufferLength / m_nFrameSize;
					hr = m_pRenderClient->GetBuffer(framesToWrite, &pData);
					if (SUCCEEDED(hr))
					{
						//
						//  Copy data from the render buffer to the output buffer and bump our render pointer.
						//
						CopyMemory(pData, renderBuffer->m_pBuffer, framesToWrite*m_nFrameSize);
						hr = m_pRenderClient->ReleaseBuffer(framesToWrite, 0);
						if (!SUCCEEDED(hr))
						{
							printf("Unable to release buffer: %x\n", hr);
							stillPlaying = false;
						}
					}
					else
					{
						printf("Unable to release buffer: %x\n", hr);
						stillPlaying = false;
					}
					//----------------------------------------------
					// generate next frame
					//-----------------------------------------------
					m_pSE->GenerateSamples(renderBuffer->m_pBuffer, m_pSE->GetParams()->GetSamplesPerBlock(), 2, 440);
					//----------------------------------------
					// Add back into queue
					//----------------------------------------
					m_pSE->m_pAudioOut->AddToRenderQueue(renderBuffer);

				}
			}
			break;
		}
	}
	//
	//  Unhook from MMCSS.
	//
	if (!DisableMMCSS)
	{
		AvRevertMmThreadCharacteristics(mmcssHandle);
	}

	CoUninitialize();
	return 0;
}


//  Called when the default render device changed.  We just want to set an event which lets the stream switch logic know that it's ok to 
//  continue with the stream switch.
//

ULONG CSynthCoreAudioSharedOut::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG CSynthCoreAudioSharedOut::Release()
{
	ULONG returnValue = InterlockedDecrement(&m_nRefCount);
	if (returnValue == 0)
	{
		delete this;
	}
	return returnValue;
}