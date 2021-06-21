#pragma once
#include "SynthObject.h"
#include "SynthDataPath.h"

class CSynthKnob :	public CSynthObject
{
	int m_MinVal;
	int m_MaxVal;
	int m_Position;
	COLORREF m_TrackColor;
	COLORREF m_KnobColor;
	COLORREF m_KnobCenter;
	COLORREF m_TickColor;
	COLORREF m_TitleColor;
	COLORREF m_TickTextColor;
	CRect m_ControlRect;
	CString m_Title;
	CString m_Ticks[11];
	CSynthDataPath * m_pKnobOut;
public:
	CSynthKnob(CSynthParameters *pP, CSynthObject *pParent);
	virtual ~CSynthKnob();
	virtual bool Create(CPoint P1,
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
	);
	//-------------------------------
	// sound generating functions
	//-------------------------------
	virtual void Run(void);
	//-------------------------------
	// drawing functions
	//-------------------------------
	virtual void Draw(CDC * pDC, CPoint Off, int Mode);
	virtual int MouseDown(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseUp(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseMove(CWnd *pWnd, int state, CPoint pos);
};

