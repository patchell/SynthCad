// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "StdAfx.h"
#include <assert.h>
#include <avrt.h>
#include "WASAPIRenderer.h"
#include "SynthEngine.h"
//
//  A simple WASAPI Render client.
//
extern RenderBuffer *renderQueue;
extern RenderBuffer **currentBufferTail ;

CWASAPIRenderer::CWASAPIRenderer(IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole) : 
    m_nRefCount(1),
    m_pEndpoint(Endpoint),
    m_pAudioClient(NULL),
    m_pRenderClient(NULL),
    m_hRenderThread(NULL),
    m_hShutdownEvent(NULL),
    m_pMixFormat(NULL),
    m_hAudioSamplesReadyEvent(NULL)
{
    m_pEndpoint->AddRef();    // Since we're holding a copy of the endpoint, take a reference to it.  It'll be released in Shutdown();
}

//
//  Empty destructor - everything should be released in the Shutdown() call.
//
CWASAPIRenderer::~CWASAPIRenderer(void) 
{
}

//
//  Initialize WASAPI in event driven mode.
//
bool CWASAPIRenderer::InitializeAudioEngine()
{
    HRESULT hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
        m_nEngineLatencyInMS*10000, 
        0, 
        m_pMixFormat, 
        NULL);

    if (FAILED(hr))
    {
        printf("Unable to initialize audio client: %x.\n", hr);
        return false;
    }

    //
    //  Retrieve the buffer size for the audio client.
    //
    hr = m_pAudioClient->GetBufferSize(&m_nBufferSize);
    if(FAILED(hr))
    {
        printf("Unable to get audio client buffer: %x. \n", hr);
        return false;
    }

    hr = m_pAudioClient->SetEventHandle(m_hAudioSamplesReadyEvent);
    if (FAILED(hr))
    {
        printf("Unable to set ready event: %x.\n", hr);
        return false;
    }

    hr = m_pAudioClient->GetService(IID_PPV_ARGS(&m_pRenderClient));
    if (FAILED(hr))
    {
        printf("Unable to get new render client: %x.\n", hr);
        return false;
    }

    return true;
}

//
//  The Event Driven renderer will be woken up every defaultDevicePeriod hundred-nano-seconds.
//  Convert that time into a number of frames.
//
UINT32 CWASAPIRenderer::BufferSizePerPeriod()
{
    REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;
    HRESULT hr = m_pAudioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
    if (FAILED(hr))
    {
        printf("Unable to retrieve device period: %x\n", hr);
        return 0;
    }
    double devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    return static_cast<UINT32>(m_pMixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

//
//  Retrieve the format we'll use to render samples.
//
//  We use the Mix format since we're rendering in shared mode.
//
bool CWASAPIRenderer::LoadFormat()
{
    HRESULT hr = m_pAudioClient->GetMixFormat(&m_pMixFormat);
    if (FAILED(hr))
    {
        printf("Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }

    m_nFrameSize = m_pMixFormat->nBlockAlign;
    if (!CalculateMixFormatType())
    {
        return false;
    }
    return true;
}

//
//  Crack open the mix format and determine what kind of samples are being rendered.
//
bool CWASAPIRenderer::CalculateMixFormatType()
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
        {
            printf("Unknown PCM integer sample type\n");
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
        printf("unrecognized device format.\n");
        return false;
    }
    return true;
}
//
//  Initialize the renderer.
//
bool CWASAPIRenderer::Initialize(UINT32 EngineLatency)
{
    if (EngineLatency < 30)
    {
        printf("Engine latency in shared mode event driven cannot be less than 30ms\n");
        return false;
    }

    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    m_hShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (m_hShutdownEvent == NULL)
    {
        printf("Unable to create shutdown event: %d.\n", GetLastError());
        return false;
    }

    m_hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (m_hAudioSamplesReadyEvent == NULL)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the renderer and thus it has to be set.
    //

    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    HRESULT hr = m_pEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_pAudioClient));
    if (FAILED(hr))
    {
        printf("Unable to activate audio client: %x.\n", hr);
        return false;
    }

//    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_DeviceEnumerator));
//    if (FAILED(hr))
//    {
//        printf("Unable to instantiate device enumerator: %x\n", hr);
//        return false;
//    }

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    if (!LoadFormat())
    {
        printf("Failed to load the mix format \n");
        return false;
    }

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //
    m_nEngineLatencyInMS = EngineLatency;

    if (!InitializeAudioEngine())
    {
        return false;
    }
    return true;
}

//
//  Shut down the render code and free all the resources.
//
void CWASAPIRenderer::Shutdown()
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
bool CWASAPIRenderer::Start(RenderBuffer *RenderBufferQueue)
{
    HRESULT hr;

//    _RenderBufferQueue = RenderBufferQueue;

    //
    //  We want to pre-roll the first buffer's worth of data into the pipeline.  That way the audio engine won't glitch on startup.  
    //
    {
        BYTE *pData;

        if (m_pSE->m_pAudioOut->m_pRenderQueue != NULL)
        {
            //
            //  Remove the buffer from the queue.
            //
            RenderBuffer *renderBuffer = m_pSE->m_pAudioOut->m_pRenderQueue;
			m_pSE->m_pAudioOut->m_pRenderQueue = renderBuffer->m_pNext;
            DWORD bufferLengthInFrames = renderBuffer->m_nBufferLength / m_nFrameSize;

            hr = m_pRenderClient->GetBuffer(bufferLengthInFrames, &pData);
            if (FAILED(hr))
            {
                printf("Failed to get buffer: %x.\n", hr);
                return false;
            }

            CopyMemory(pData, renderBuffer->m_pBuffer, renderBuffer->m_nBufferLength);
            hr = m_pRenderClient->ReleaseBuffer(bufferLengthInFrames, 0);

            delete renderBuffer;
        }
        else
        {
            hr = m_pRenderClient->GetBuffer(m_nBufferSize, &pData);
            if (FAILED(hr))
            {
                printf("Failed to get buffer: %x.\n", hr);
                return false;
            }
            hr = m_pRenderClient->ReleaseBuffer(m_nBufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
        }
        if (FAILED(hr))
        {
            printf("Failed to release buffer: %x.\n", hr);
            return false;
        }
    }

    //
    //  Now create the thread which is going to drive the renderer.
    //
    m_hRenderThread = CreateThread(NULL, 0, WASAPIRenderThread, this, 0, NULL);
    if (m_hRenderThread == NULL)
    {
        printf("Unable to create transport thread: %x.", GetLastError());
        return false;
    }

    //
    //  We're ready to go, start rendering!
    //
    hr = m_pAudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start render client: %x.\n", hr);
        return false;
    }

    return true;
}

//
//  Stop the renderer.
//
void CWASAPIRenderer::Stop()
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
    {
        printf("Unable to stop audio client: %x\n", hr);
    }

    if (m_hRenderThread)
    {
        WaitForSingleObject(m_hRenderThread, INFINITE);

        CloseHandle(m_hRenderThread);
        m_hRenderThread = NULL;
    }

    //
    //  Drain the buffers in the render buffer queue.
    //
   while (m_pSE->m_pAudioOut->m_pRenderQueue != NULL)
   {
       RenderBuffer *renderBuffer = m_pSE->m_pAudioOut->m_pRenderQueue;
	   m_pSE->m_pAudioOut->m_pRenderQueue = renderBuffer->m_pNext;
       delete renderBuffer;
    }

}


//
//  Render thread - processes samples from the audio engine
//
DWORD CWASAPIRenderer::WASAPIRenderThread(LPVOID Context)
{
    CWASAPIRenderer *renderer = static_cast<CWASAPIRenderer *>(Context);
    return renderer->DoRenderThread();
}

DWORD CWASAPIRenderer::DoRenderThread()
{
    bool stillPlaying = true;
    HANDLE waitArray[2] = {m_hShutdownEvent, m_hAudioSamplesReadyEvent};
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    if (!DisableMMCSS)
    {
        mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
        if (mmcssHandle == NULL)
        {
            printf("Unable to enable MMCSS on render thread: %d\n", GetLastError());
        }
    }

    while (stillPlaying)
    {
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // m_hShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
            //
            //  We need to provide the next buffer of samples to the audio renderer.
            //
            BYTE *pData;
            UINT32 padding;
            UINT32 framesAvailable;
            //
            //  We want to find out how much of the buffer *isn't* available (is padding).
            //
            hr = m_pAudioClient->GetCurrentPadding(&padding);
            if (SUCCEEDED(hr))
            {
                //
                //  Calculate the number of frames available.  We'll render
                //  that many frames or the number of frames left in the buffer, whichever is smaller.
                //
                framesAvailable = m_nBufferSize - padding;

                //
                //  If the buffer at the head of the render buffer queue fits in the frames available, render it.  If we don't
                //  have enough room to fit the buffer, skip this pass - we will have enough room on the next pass.
                //
                if (m_pSE->m_pAudioOut->m_pRenderQueue == NULL)
                {
                    stillPlaying = false;
                }
                else if (m_pSE->m_pAudioOut->m_pRenderQueue->m_nBufferLength <= (framesAvailable *m_nFrameSize))
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

ULONG CWASAPIRenderer::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}
ULONG CWASAPIRenderer::Release()
{
    ULONG returnValue = InterlockedDecrement(&m_nRefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}