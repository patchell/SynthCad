#include "stdafx.h"
#include "SynthFmOp.h"
#include "SynthCadDefines.h"

CSynthFmOp::CSynthFmOp(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_FMOP,pParams,pParent)
{
	m_pAtackSlider = 0;
	m_pAttackLevelSlider = 0;
	m_pDecaySlider = 0;
	m_pDecayLevelSlider = 0;
	m_pReleaseSlider = 0;
	m_pOut = 0;	//oscillator output
	m_pPitch = 0;	//pitch control
	m_pPitchMultiplier = 0;
	m_pPitchDirect = 0;	//direct pitch control (linear)
	m_pAttack = 0;
	m_pAttackLevel = 0;
	m_pDecay = 0;
	m_pDecayLevel = 0;
	m_pRelease = 0;
	m_pTrig = 0;
}


CSynthFmOp::~CSynthFmOp()
{
}

void CSynthFmOp::Run(void)
{
	//----------------------------
	// Run
	//		This methode does all
	// of the sound synth stuff.
	// It has two major components
	// 1. Sine Oscillator
	// 2. Envelope Generator
	// 3. VCA....(minor)
	//----------------------------
	float TimeConstant = 1.0;
	float Level = 0.0;
	float o;	//output value

	//----------------------------
	// calculate pitch
	//	pitch is determined by the
	// sum of two inputs.  The first
	// input is the pitch that is
	// calculated based on the
	// equally tempered scale and
	// the second is the FM modulation.
	//-----------------------------
	float pitch = m_pPitch->GetData();
	if (m_pPitchMultiplier) pitch *= m_pPitchMultiplier->GetData() * 10;
	float OscFreqInc = FreqInc(LevelToFreq(pitch));
	m_Phase += OscFreqInc;
	if (m_pPitchDirect)
	{
		m_Phase += m_pPitchDirect->GetData();
	}
	while (m_Phase > 1.0)m_Phase -= 1.0;

	//---------------------------------------
	// Envelope Generator
	//---------------------------------------

	//---------------------------------------
	// State Machine for ADSR Envelope
	// Generator
	//---------------------------------------
	switch (m_EVState)
	{
	case FMOP_EGSTATE_RELEASE:
		if (m_pTrig->GetData() > 0.5)
			m_EVState = FMOP_EGSTATE_ATTACK;
		Level = 0.0;
		TimeConstant = m_pRelease->GetData();
		break;
	case FMOP_EGSTATE_ATTACK:
		if (fabs(m_ZEnvelope - m_pAttackLevel->GetData()) < 0.001)m_EVState = FMOP_EGSTATE_DECAY;
		else if (m_pTrig->GetData() < 0.5)
			m_EVState = FMOP_EGSTATE_RELEASE;
		Level = (float)1.1;
		TimeConstant = m_pAttack->GetData();
		break;
	case FMOP_EGSTATE_DECAY:
		if (m_pTrig->GetData() < 0.5)
			m_EVState = FMOP_EGSTATE_RELEASE;
		Level = m_pDecayLevel->GetData();
		TimeConstant = m_pDecay->GetData();
		break;
	}
	//------------------------------------------
	// The core of the EG is a one pole filter
	//------------------------------------------
	float Fc = LevelToFC(TimeConstant);
	m_ZEnvelope = m_ZEnvelope + (double(Level) - m_ZEnvelope) *double(Fc);
	o = (float)sin(twoPI *  m_Phase) * m_ZEnvelope;	//VCA
	if (m_pOut) m_pOut->SetData(o);
}

float CSynthFmOp::FreqInc(float freq)
{
	return freq / this->GetParams()->GetSampleRate();
}

bool CSynthFmOp::Create(CPoint P1)
{
	m_FillColor = RGB(128, 128, 256);
	m_LineColor = RGB(128, 0, 0);
	m_Outline.SetRect(CPoint(0, 0), CPoint(100, 400));
	// Create sliders

	return CSynthObject::Create(P1);
}


float CSynthFmOp::TimeToLevel(float time)
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
	Level = float(log(time / 1.024) / (log(TWELTHROOT2) * 120));
	if (Level > 1.0) Level = float(1.0);
	else if (Level < -1.0) Level = float(1.0);
	return Level;
}

float CSynthFmOp::LevelToTime(float Level)
{
	float Time;
	Time = float(1.024 * pow(TWELTHROOT2, Level * 120.0));
	return Time;
}

float CSynthFmOp::LevelToFC(float level)
{
	float Fc;
	float Time = LevelToTime(level);
	Fc = float(twoPI / (Time* this->GetParams()->GetSampleRate()));
	return Fc;
}

void CSynthFmOp::Draw(CDC *pDC, CPoint Off, int mode)
{

}

int CSynthFmOp::MouseDown(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

int CSynthFmOp::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

int CSynthFmOp::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	return state;
}

void CSynthFmOp::Move(CPoint p)
{

}

void CSynthFmOp::Save(FILE * pO, int Indent, int flags )
{

}

int CSynthFmOp::Parse(int Token, CLexer *pLex)
{
	return 0;
}

int CSynthFmOp::CheckSelected(CPoint p, CSynthObject ** ppSelList , int index, int n)
{
	return 0;
}

void CSynthFmOp::AdjustReference(CPoint p)
{

}

CRect CSynthFmOp::GetRect(void)
{
	return CRect();
}

