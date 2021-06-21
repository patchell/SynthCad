// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#pragma once
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
#include "RenderBuffer.h"

//#include "SynthEngine.h"
class CSynthEngine;

class CWASAPIRenderer
{
public:
	CSynthEngine * m_pSE;
    //  Public interface to CWASAPIRenderer.
    enum RenderSampleType
    {
        SampleTypeFloat,
        SampleType16BitPCM,
    };
	int GetSampleType() {return m_eRenderSampleType;}
    CWASAPIRenderer(IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole);
	~CWASAPIRenderer(void);
	bool Initialize(UINT32 EngineLatency);
    void Shutdown();
    bool Start(RenderBuffer *RenderBufferQueue);
    void Stop();
    WORD ChannelCount() { return m_pMixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return m_pMixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return m_pMixFormat->wBitsPerSample / 8; }
    RenderSampleType SampleType() { return m_eRenderSampleType; }
    UINT32 FrameSize() { return m_nFrameSize; }
    UINT32 BufferSize() { return m_nBufferSize; }
    UINT32 BufferSizePerPeriod();
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

private:
    LONG    m_nRefCount;
    //
    //  Core Audio Rendering member variables.
    //
    IMMDevice * m_pEndpoint;
    IAudioClient *m_pAudioClient;
    IAudioRenderClient *m_pRenderClient;

    HANDLE      m_hRenderThread;
    HANDLE      m_hShutdownEvent;
    HANDLE      m_hAudioSamplesReadyEvent;
    WAVEFORMATEX *m_pMixFormat;
    UINT32      m_nFrameSize;
    RenderSampleType m_eRenderSampleType;
    UINT32      m_nBufferSize;
    LONG        m_nEngineLatencyInMS;

    //
    //  Render buffer management.
    //
//    RenderBuffer *_RenderBufferQueue;

    static DWORD __stdcall WASAPIRenderThread(LPVOID Context);
    DWORD CWASAPIRenderer::DoRenderThread();
    //
    //  Stream switch related members and methods.
    //
    //
    //  Utility functions.
    //
    bool CalculateMixFormatType();
    bool InitializeAudioEngine();
    bool LoadFormat();
};
