#include "stdafx.h"
#include "SynthFmMod.h"


CSynthFmMod::CSynthFmMod(CSynthParameters *pParam, CSynthObject *pParent)
		:CSynthObject(OBJECT_TYPE_FMMODULE, pParam,pParent)
{
	m_pOp1 = m_pOp2 = m_pOp3 = m_pOp4 = 0;
	m_pD1 = m_pD2 = m_pD3 = m_pD4 = 0;
	m_pSummer = 0;
	m_pPitch = 0;
	m_pTrig = 0;
	m_pOut = 0;
}


CSynthFmMod::~CSynthFmMod()
{
}

void CSynthFmMod::Run(void)
{
	CSynthObject *pO = GetHead();
	while (pO)
	{
		pO->Run();
		pO = pO->GetNext();
	}
}

void CSynthFmMod::PatchAlgorithm(int algo)
{
	if (m_pSummer)
	{
		RemoveObject(m_pSummer);
		delete m_pSummer;
		m_pSummer = 0;
	}

	switch (algo)
	{
	case 0:
		//---------------------------------
		//         ____
		// ___   _|_   |
		//| 4 | | 2 |  |
		//|___| |___|  |
		//  |     |____|
		// _|_   _|_
		//| 3 | | 1 |
		//|___| |___|
		//  |_____| 
		//---------------------------------
		m_pSummer = new CSynthSummer(2, 0.5, GetParams(), this);
		AddObject(m_pSummer);
		m_pOp1->SetPitchDirect(m_pD1);
		m_pOp2->SetOut(m_pD1);
		m_pOp2->SetPitchDirect(m_pD1);
		m_pOp4->SetOut(m_pD2);
		m_pOp3->SetPitchDirect(m_pD2);
		m_pOp3->SetOut(m_pD3);
		m_pOp1->SetOut(m_pD4);
		m_pSummer->SetIn(0, m_pD3);
		m_pSummer->SetIn(1, m_pD4);
		m_pSummer->SetOut(m_pOut);
		m_pOp1->SetPitch(m_pPitch);
		m_pOp2->SetPitch(m_pPitch);
		m_pOp3->SetPitch(m_pPitch);
		m_pOp4->SetPitch(m_pPitch);
		break;
	case 1:
		//---------------------------------
		//         ____
		// ___   _|_   |
		//| 4 | | 2 |  |
		//|___| |___|  |
		//  |     |    |
		// _|_   _|_   |
		//| 3 | | 1 |  |
		//|___| |___|  |
		//  |     |____|
		//  |_____| 
		//---------------------------------
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	}
}

bool CSynthFmMod::Create(CPoint p)
{
	m_pD1 = new CSynthDataPath(GetParams(), this);
	m_pD2 = new CSynthDataPath(GetParams(), this);
	m_pD3 = new CSynthDataPath(GetParams(), this);
	m_pD4 = new CSynthDataPath(GetParams(),this);
	m_pOp1 = new CSynthFmOp(GetParams(), this);
	m_pOp1->Create(p);
	AddObject(m_pOp1);
	m_pOp2 = new CSynthFmOp(GetParams(), this);
	m_pOp2->Create(p);
	AddObject(m_pOp2);
	m_pOp3 = new CSynthFmOp(GetParams(), this);
	m_pOp3->Create(p);
	AddObject(m_pOp3);
	m_pOp4 = new CSynthFmOp(GetParams(), this);
	m_pOp4->Create(p);
	AddObject(m_pOp4);
	PatchAlgorithm(0);	//default algorithm
	return CSynthObject::Create(p);
}