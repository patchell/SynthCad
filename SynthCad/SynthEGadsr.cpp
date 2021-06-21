#include "stdafx.h"
#include <math.h>
#include "SynthCadDefines.h"
#include "SynthEGadsr.h"

CSynthEGadsr::CSynthEGadsr(CSynthParameters *Params, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_EGADSR,Params,pParent)
{
	m_Z = 0.0;
	m_pTrigger = 0;
	m_pAttack = 0;
	m_pDecay = 0;
	m_pOut = 0;
	m_pRelease = 0;
	m_pSustain = 0;
	m_State = EGSTATE_RELEASE;
}

CSynthEGadsr::~CSynthEGadsr()
{
	//dtor
}

void CSynthEGadsr::Run(void)
{
	float TimeConstant=1.0;
	float Level=0.0;

	//---------------------------------------
	// State Machine for ADSR Envelope
	// Generator
	//---------------------------------------
	switch (m_State)
	{
	case EGSTATE_RELEASE:
		if (m_pTrigger->GetData() > 0.5)
			m_State = EGSTATE_ATTACK;
		Level = 0.0;
		TimeConstant = m_pRelease->GetData();
		break;
	case EGSTATE_ATTACK:
		if (m_Z > 0.99)m_State = EGSTATE_DECAY;
		else if (m_pTrigger->GetData() < 0.5)
			m_State = EGSTATE_RELEASE;
		Level = (float)1.1;
		TimeConstant = m_pAttack->GetData();
		break;
	case EGSTATE_DECAY:
		if (m_pTrigger->GetData() < 0.5)
			m_State = EGSTATE_RELEASE;
		Level = m_pSustain->GetData();
		TimeConstant = m_pDecay->GetData();
		break;
	}
	//------------------------------------------
	// The core of the EG is a one pole filter
	//------------------------------------------
	float Fc = LevelToFC(TimeConstant);
	m_Z = m_Z + (double(Level) - m_Z) *double( Fc);
	m_pOut->SetData((float)m_Z);
}

float CSynthEGadsr::TimeToLevel(float time)
{
	///-----------------------------------------------
	/// TimeToLevel
	///
	/// 1.0 -> .001 Seconds
	/// 0.0 -> 1.024 Seconds
	/// -1.0 -> 1048 Seconds
	/// parameters:
	///		time.......tim constant in seconds
	///------------------------------------------------
	float Level;
	Level = float(log(time/1.024) / (log(TWELTHROOT2) * 120));
	if (Level > 1.0) Level = float(1.0);
	else if (Level < -1.0) Level = float(1.0);
	return Level;
}

float CSynthEGadsr::LevelToTime(float Level)
{
	float Time;
	Time =float( 1.024 * pow(TWELTHROOT2,Level * 120.0));
	return Time;
}

float CSynthEGadsr::LevelToFC(float level)
{
	float Fc;
	float Time = LevelToTime(level);
	Fc = float(twoPI / (Time* this->GetParams()->GetSampleRate()));
	return Fc;
}

bool CSynthEGadsr::Create(CPoint p1, int Am, int Dm, int Sm, int Rm)
{
	//---------------------------------------------
	// Create
	//		This method creates the envelope gen
	// and all of its components
	//
	// parameters:
	//		p1......reference point
	//		Am......Message to send attack data
	//		Dm......Message to send decay data
	//		Sm......Message to send sustain data
	//		Rm......Message to send release data
	//---------------------------------------------
	m_Outline.SetRect(CPoint(0, 0), CPoint(200, 400));
	m_LineColor = RGB(128, 0, 0);
	m_FillColor = RGB(128, 128, 255);
	//----------------------------------
	// add sliders
	//-----------------------------------
	m_pAttackCtl = new CSynthSlider(GetParams(), this);
	m_pAttackCtl->Create(CPoint(30, 50), CPoint(50, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(255, 0, 255));
	m_pAttackCtl->SetMessageValue(Am);
	AddObject(m_pAttackCtl);
	m_pDecayCtl = new CSynthSlider(GetParams(), this);
	m_pDecayCtl->Create(CPoint(70, 50), CPoint(90, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(255, 0, 255));
	m_pDecayCtl->SetMessageValue(Dm);
	AddObject(m_pDecayCtl);
	m_pSustainCtl = new CSynthSlider(GetParams(), this);
	m_pSustainCtl->Create(CPoint(110, 50), CPoint(130, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(255, 0, 255));
	m_pSustainCtl->SetMessageValue(Sm);
	AddObject(m_pSustainCtl);
	m_pReleaseCtl = new CSynthSlider(GetParams(), this);
	m_pReleaseCtl->Create(CPoint(150, 50), CPoint(170, 350), this, SYNTHSLIDER_VERT, RGB(0, 0, 0), RGB(0, 0, 255), RGB(255, 0, 255));
	m_pReleaseCtl->SetMessageValue(Rm);
	AddObject(m_pReleaseCtl);
	return CSynthObject::Create(p1);
}

//-------------------------------
// Drawing Functions
//-------------------------------
void CSynthEGadsr::Draw(CDC *pDC,CPoint Off,int mode)
{
	CPen pen, *OldPen;
	CBrush br, *OldBr;
	CRect rect = m_Outline;

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
		pObj->Draw(pDC,GetReference(), mode);
		pObj = pObj->GetNext();
	}
}

int CSynthEGadsr::MouseDown(CWnd *pWnd, int state, CPoint pos)
{
	CSynthObject *pObj;
	pObj = GetHead();
	while (pObj)
	{
		state = pObj->MouseDown(pWnd, state,pos-GetReference());
		pObj = pObj->GetNext();
	}
	return state;
}

int CSynthEGadsr::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	CSynthObject *pObj;
	pObj = GetHead();
	while (pObj)
	{
		state = pObj->MouseUp(pWnd, state, pos - GetReference());
		pObj = pObj->GetNext();
	}
	return state;
}

int CSynthEGadsr::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	CSynthObject *pObj;
	pObj = GetHead();
	while (pObj)
	{
		state = pObj->MouseMove(pWnd, state, pos - GetReference());
		pObj = pObj->GetNext();
	}
	return state;
}

void CSynthEGadsr::Move(CPoint p)
{

}

void CSynthEGadsr::Save(FILE * pO, int Indent, int flags)
{

}

int CSynthEGadsr::Parse(int Token, CLexer *pLex)
{
	return Token;
}

int CSynthEGadsr::CheckSelected(CPoint p, CSynthObject ** ppSelList, int index, int n)
{
	return 0;
}

void CSynthEGadsr::AdjustReference(CPoint p)
{

}

CRect CSynthEGadsr::GetRect(void)
{
	return m_Outline;
}

