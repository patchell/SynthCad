#pragma once

#include "SynthCadDefines.h"
#include "SynthObject.h"
#include "SynthDataPath.h"
#include "SynthParameters.h"
#include "WASAPIRenderer.h"
#include "RenderBuffer.h"

class CSynthAudioOut:public CSynthObject,public CWASAPIRenderer
{
	//data input
	CSynthDataPath *m_pIN;
	float m_Data;
public:
	inline void AddToRenderQueue(RenderBuffer *pRB) {
		*m_ppRenderQueueTail = pRB;
		m_ppRenderQueueTail = &pRB->m_pNext;
	}

	RenderBuffer *m_pRenderQueue;
	RenderBuffer **m_ppRenderQueueTail;
	CSynthAudioOut(CSynthParameters *pParams, CSynthObject *pParent, IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole);
	virtual ~CSynthAudioOut();
	virtual bool Create( CPoint p1);
	virtual void Run();
	void Open();
	void Close();
	//----------------------------------
	// Getter Functions
	//----------------------------------
	inline void SetDataInput(CSynthDataPath *pD) { m_pIN = pD; }
	inline float GetData(void) { return m_Data; }
	void RunSynthEngine(short *pB);
};

