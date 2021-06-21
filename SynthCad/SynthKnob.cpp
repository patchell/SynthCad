#include "stdafx.h"
#include "SynthKnob.h"


CSynthKnob::CSynthKnob(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_KNOB, pParams,pParent)
{
	m_MinVal = 0;
	m_MaxVal = 0;
	m_Position = 0;
}


CSynthKnob::~CSynthKnob()
{
}


void CSynthKnob::Run(void)
{
}

bool CSynthKnob::Create(
	CPoint P1,
	CPoint P2,
	CSynthObject *pParent,
	unsigned stye,
	COLORREF cBackground,
	COLORREF cTrack,
	COLORREF cKnob,
	COLORREF cKnobCenter,
	COLORREF cTicks,
	COLORREF cTitle,
	COLORREF cTickText
)
{
	return true;
}
void CSynthKnob::Draw(CDC * pDC, CPoint Off, int Mode)
{

}
int CSynthKnob::MouseDown(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}
int CSynthKnob::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}
int CSynthKnob::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}
