#pragma once
#include "SynthCadDefines.h"
#include "SynthObject.h"
#include "SynthDataPath.h"
#include "SynthParameters.h"
#include "RenderBuffer.h"

class CSynthEngine;


class CSynthCoreAudioSharedOut :public CSynthObject
{
public:
	enum RenderSampleType
	{
		SampleTypeFloat,
		SampleType16BitPCM,
	};
private:
	//data input
	CSynthDataPath *m_pIN;
	float m_Data;
	//--------------------------------------------
	//  Core Audio Rendering member variables.
	//--------------------------------------------
	IMMDevice * m_pEndpoint;
	IAudioClient *m_pAudioClient;
	IAudioRenderClient *m_pRenderClient;
	UINT32 m_nRefCount;
	HANDLE      m_hRenderThread;
	HANDLE      m_hShutdownEvent;
	HANDLE      m_hAudioSamplesReadyEvent;
	WAVEFORMATEX *m_pMixFormat;
	UINT32      m_nFrameSize;
	RenderSampleType m_eRenderSampleType;
	UINT32      m_nBufferSize;
	LONG        m_nEngineLatencyInMS;
public:
	CSynthEngine * m_pSE;
	RenderBuffer *m_pRenderQueue;
	RenderBuffer **m_ppRenderQueueTail;
	inline void AddToRenderQueue(RenderBuffer *pRB) {
		*m_ppRenderQueueTail = pRB;
		m_ppRenderQueueTail = &pRB->m_pNext;
	}

	//  Public interface to CWASAPIRenderer.

public:
	CSynthCoreAudioSharedOut(CSynthParameters *pParams, CSynthObject *pParent, IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole);
	virtual ~CSynthCoreAudioSharedOut();
	virtual bool Create(CPoint p1);
	virtual void Run();
	void Open();
	void Close();
	//----------------------------------
	// Getter Functions
	//----------------------------------
	int GetSampleType() { return m_eRenderSampleType; }
	inline void SetDataInput(CSynthDataPath *pD) { m_pIN = pD; }
	inline float GetData(void) { return m_Data; }
	void RunSynthEngine(short *pB);
	//----------------------------------
	// Render Methods
	//----------------------------------
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
	//----------------------------
	//  Utility functions.
	//----------------------------56ilouyh7
	bool CalculateMixFormatType();
	bool InitializeAudioEngine();
	bool LoadFormat();
	//-----------------------------------
	// worker thread methods
	//-----------------------------------
	static DWORD WASAPIRenderThread(LPVOID param);
	DWORD DoRenderThread();
};

