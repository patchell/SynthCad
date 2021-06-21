#pragma once
#include "SynthObject.h"
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
//#include "SynthEngine.h"
#include "SynthDataPath.h"
#include "RenderBuffer.h"

class CSynthEngine;


class CSynthCoreAudioOutput : public CSynthObject
{
	CSynthDataPath *m_pIN;
	HANDLE m_hRenderThread;
	HANDLE m_hShutdownEvent;
	HANDLE m_hAudioSamplesReadyEvent;
	IMMDevice * m_pEndpoint;
	IAudioClient *m_pAudioClient;
	IAudioRenderClient *m_pRenderClient;
	WAVEFORMATEX *m_pMixFormat;
	UINT32      m_nFrameSize;
	RenderSampleType m_eRenderSampleType;
	UINT32      m_nBufferSize;
	UINT32      m_nBufferSizePerPeriod;
	LONG        m_nEngineLatencyInMS;
	//
	//  Render buffer management.
	//
	RenderBuffer *m_pRenderBufferQueue;
	RenderBuffer **m_ppRendBuffQueueTail;
	float m_Data;
public:
	CSynthCoreAudioOutput(CSynthParameters *pParams, CSynthObject *pParent, IMMDevice *Endpoint);
	virtual ~CSynthCoreAudioOutput();
	virtual bool Create(int DevID,int Latency);
	virtual bool Open();
	virtual void Close();
	bool Start();
	void Stop();
	static UINT RenderThread(LPVOID param);
	bool StartThread();
	bool TerminateThread();
	bool DisplayError(LPCWSTR Message, int Error);
	bool LoadFormat();
	bool CalculateMixFormatType();
	bool InitializeAudioEngine();
	bool DisplayInfo(LPCWSTR Message, int Value);
	WORD ChannelCount() { return m_pMixFormat->nChannels; }
	UINT32 SamplesPerSecond() { return m_pMixFormat->nSamplesPerSec; }
	UINT32 BytesPerSample() { return m_pMixFormat->wBitsPerSample / 8; }
	RenderSampleType SampleType() { return m_eRenderSampleType; }
	UINT32 FrameSize() { return m_nFrameSize; }
	UINT32 BufferSize() { return m_nBufferSize; }
	UINT32 BufferSizePerPeriod() { return m_nBufferSizePerPeriod; }
	UINT DoRenderThread();
	CSynthEngine *GetSE(void) { return (CSynthEngine *)GetParent(); }
	virtual void Run();
	float GetData() { return m_Data; }
	void RunSynthEngine();
	inline void SetDataInput(CSynthDataPath *pD) { m_pIN = pD; }
	inline CSynthDataPath *GetDataInput() { return m_pIN; }
	int GetSampleType() { return m_eRenderSampleType; }
};

