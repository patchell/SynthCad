#pragma once
#include "SynthObject.h"
#include "SynthFmOp.h"
#include "SynthParameters.h"
#include "SynthDataPath.h"
#include "SynthSummer.h"

class CSynthFmMod :	public CSynthObject
{
	CSynthDataPath *m_pD1, *m_pD2, *m_pD3, *m_pD4;
	CSynthDataPath *m_pPitch;
	CSynthDataPath *m_pTrig;
	CSynthDataPath *m_pOut;
	CSynthFmOp *m_pOp1;
	CSynthFmOp *m_pOp2;
	CSynthFmOp *m_pOp3;
	CSynthFmOp *m_pOp4;
	CSynthSummer *m_pSummer;
	int m_Alg;	//how the operators are interconnected
	int m_nSummer;	//number of items to sum
	float m_SumFactor;	//scale each sum input by this
public:
	CSynthFmMod(CSynthParameters *pParam,CSynthObject *pParent);
	virtual ~CSynthFmMod();
	virtual void Run(void);
	virtual bool Create(CPoint p);
	void PatchAlgorithm(int algo);
};

