#include "stdafx.h"
#include "SynthCadDefines.h"
#include "SynthSVFilter.h"

CSynthSVFilter::CSynthSVFilter(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_SVFILTER,pParams,pParent)
{
	m_Z1 = 0.0;
	m_Z2 = 0.0;
	m_pIn = 0;
	m_pHPo = 0;
	m_pBPo = 0;
	m_pLPo = 0;
	m_pFc = 0;
	m_pQ = 0;
}

CSynthSVFilter::~CSynthSVFilter()
{
}

bool CSynthSVFilter::Create(CPoint p1)
{
	m_OutLine.SetRect(CPoint(0, 0), CPoint(150, 400));
	m_FillColor = RGB(128, 0, 255);
	m_LineColor = RGB(128, 0, 0);
	// Add slider controls
	m_pFreqCtl = new CSynthSlider(GetParams(), this);
	m_pFreqCtl->Create(CPoint(28, 50), CPoint(48, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(129,0, 255));
	AddObject(m_pFreqCtl);
	m_pFreqAtnCtl = new CSynthSlider(GetParams(), this);
	m_pFreqAtnCtl->Create(CPoint(66, 50), CPoint(86, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(129, 0, 255));
	AddObject(m_pFreqAtnCtl);
	m_pQCtl = new CSynthSlider(GetParams(), this);
	m_pQCtl->Create(CPoint(104, 50), CPoint(124, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(129, 0, 255));
	AddObject(m_pQCtl);
	return CSynthObject::Create(p1);
}

void CSynthSVFilter::Run()
{
	///--------------------------------------------
	/// Run
	///		Implements the Hal Chamberlin State
	///	variable digital filter.
	/// Page 489 of Musical Applications of Micro Processors
	///---------------------------------------------
	float tZ1,tZ2;
	float Fc,Q;
	float FiltIn;
	float Hp;

	FiltIn = m_pIn->GetData();
	Fc = LevelToFreq(m_pFc->GetData());
	Fc = this->FreqToFc(Fc);
	Q = m_pQ->GetData();

	tZ2 = Integrator(Fc,m_Z2,m_Z1);
	Hp = Q * FiltIn - Q * m_Z1 - m_Z2;
	tZ1 = Integrator(Fc,m_Z1,Hp);
	m_Z1 = tZ1;
	m_Z2 = tZ2;

	if(m_pHPo)
		m_pHPo->SetData(Hp);
	if(m_pBPo)
		m_pBPo->SetData(m_Z1);
	if(m_pLPo)
		m_pLPo->SetData(m_Z2);
}

inline float CSynthSVFilter::Integrator(float fc, float Z, float In)
{
	float rV = fc * In + Z;
	if (rV > 1.0) rV = 1.0;
	else if (rV < -1.0) rV = -1.0;
	return rV;
}

float CSynthSVFilter::FreqToFc(float freq)
{
	///---------------------------------------------
	/// FreqToFc
	///		Function converts frequency to a control
	/// value which is always POSITIVE.  Negative
	/// values will do BAD things
	///---------------------------------------------
	float Fc;

	Fc =(float)( twoPI * freq/this->GetParams()->GetSampleRate());
	return Fc;
}

//-------------------------------
// Drawing Functions
//-------------------------------
void CSynthSVFilter::Draw(CDC *pDC, CPoint Off, int mode)
{
	CPen pen, *OldPen;
	CBrush br, *OldBr;
	CRect rect = m_OutLine;

	rect.OffsetRect(GetReference());
	br.CreateSolidBrush(m_FillColor);
	pen.CreatePen(PS_SOLID, 3, m_LineColor);
	OldPen = pDC->SelectObject(&pen);
	OldBr = pDC->SelectObject(&br);
	pDC->Rectangle(&rect);
	pDC->SelectObject(OldBr);
	pDC->SelectObject(OldPen);

	CSynthObject *pObj = GetHead();
	while (pObj)
	{
		pObj->Draw(pDC, GetReference(), mode);
		pObj = pObj->GetNext();
	}
}

int CSynthSVFilter::MouseDown(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

int CSynthSVFilter::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

int CSynthSVFilter::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

void CSynthSVFilter::Move(CPoint p)
{

}

void CSynthSVFilter::Save(FILE * pO, int Indent, int flags)
{

}

int CSynthSVFilter::Parse(int Token, CLexer *pLex)
{
	return Token;
}

int CSynthSVFilter::CheckSelected(CPoint p, CSynthObject ** ppSelList, int index, int n)
{
	return 0;
}

void CSynthSVFilter::AdjustReference(CPoint p)
{

}

CRect CSynthSVFilter::GetRect(void)
{
	return m_OutLine;
}

