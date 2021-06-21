#pragma once
#include "SynthObject.h"
#include "SynthDataPath.h"

class CSynthSummer :public CSynthObject
{
	int m_nChannels;
	CSynthDataPath **m_ppIn;
	CSynthDataPath *m_pOut;
	float m_Factor;
public:
	CSynthSummer(int nChan, float factor,CSynthParameters *pParams, CSynthObject *pParent);
	virtual ~CSynthSummer();
	virtual bool Create(CPoint p);
	virtual void Run(void);
	inline CSynthDataPath *GetIn(int ch) { return m_ppIn[ch]; }
	inline void SetIn(int ch, CSynthDataPath *val) { m_ppIn[ch] = val; }
	inline CSynthDataPath *GetOut() { return m_pOut; }
	inline void SetOut(CSynthDataPath *val) { m_pOut = val; }
};

