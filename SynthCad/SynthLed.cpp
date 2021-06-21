#include "stdafx.h"
#include "SynthLed.h"
#include "SynthParameters.h"

CSynthLed::CSynthLed(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_LED, pParams,pParent)
{
	M_ColorLevel = 1.0;
}


CSynthLed::~CSynthLed()
{
}

void CSynthLed::Run(void)
{
	//-----------------------------------
	// Run
	//		This method can also change
	//	the LED by reading the input
	//	data.
	//-----------------------------------
	if (m_pLedDataIn)
	{
		float v = m_pLedDataIn->GetData();
	}
}


bool CSynthLed::Create(CPoint p1)
{
	return CSynthObject::Create(p1);
}

void CSynthLed::Draw(CDC *pDC, CPoint Off, int mode)
{
	CRect rect = GetRect();
	CPen pen, *oldpen;
	CBrush br, *oldbr;
	COLORREF color = (int)(m_LedColor * M_ColorLevel);
	pen.CreatePen(PS_SOLID, 1, color);
	br.CreateSolidBrush(color);
	oldpen = pDC->SelectObject(&pen);
	oldbr = pDC->SelectObject(&br);
	pDC->Ellipse(rect);
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbr);
	
}

int CSynthLed::MouseDown(CWnd *pWnd, int state, CPoint pos)
{
	return state; 
}

int CSynthLed::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	return state; 
}

int CSynthLed::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	return state; 
}

void CSynthLed::Move(CPoint p)
{

}

void CSynthLed::Save(FILE * pO, int Indent, int flags)
{

}

int CSynthLed::Parse(int Token, CLexer *pLex)
{
	return Token;
}

int CSynthLed::CheckSelected(CPoint p, CSynthObject ** ppSelList , int index , int n)
{
	return 0;
}

CPoint CSynthLed::GetReference()
{
	return CPoint();
}

void CSynthLed::AdjustReference(CPoint p)
{

}

