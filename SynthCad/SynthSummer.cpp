#include "stdafx.h"
#include "SynthSummer.h"
#include "SynthParameters.h"

CSynthSummer::CSynthSummer(int nChan, float factor, CSynthParameters *pParams,CSynthObject *pParent):CSynthObject(OBJECT_TYPE_SUMMER,pParams,pParent)
{
	m_nChannels = nChan;
	m_ppIn = new CSynthDataPath *[m_nChannels];
	m_Factor = factor;
}


CSynthSummer::~CSynthSummer()
{
	delete[] m_ppIn;
}

bool CSynthSummer::Create(CPoint p)
{
	return CSynthObject::Create(p);
}

void CSynthSummer::Run(void)
{
	int i;
	float o = 0.0;
	for (i = 0; i < m_nChannels; ++i)
		o += m_ppIn[i]->GetData() * m_Factor;
	m_pOut->SetData(o);
}
