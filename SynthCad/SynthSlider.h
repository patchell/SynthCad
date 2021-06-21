#pragma once
#include "SynthObject.h"
#include "SynthDataPath.h"
#include "Knob.h"
#define SYNTHSLIDER_HORZ		1
#define SYNTHSLIDER_VERT		2

class CSynthSlider :public CSynthObject
{
	CRect m_objRect;
	CRect m_Track;
	CKnob m_Knob;
	unsigned m_style;
	CSynthDataPath *m_pSliderOut;
	int m_MaxVal;
	int m_MinVal;
	int m_Value;
	COLORREF m_Color;
	COLORREF m_TrackColor;
	COLORREF m_KnobColor;
	CPoint m_KnobSartDragPos;
	int m_Dragging;
	int m_Message;
public:
	//--------------------------------------
	// Getter Functions
	//--------------------------------------
	inline int GetMessageValue() { return m_Message; }
	inline void SetMessageValue(int m) { m_Message = m; }
	inline int GetValue() { return m_Value; }
	inline void SetValue(int v) { m_Value = v; }
	//--------------------------------------
	// Implementation
	//--------------------------------------
	CSynthSlider(CSynthParameters *pParm, CSynthObject *pParent);
	virtual ~CSynthSlider();
	int Position();
	virtual bool Create(CPoint P1,
		CPoint P2,
		CSynthObject *pParent,
		unsigned stye,
		COLORREF cBackground,
		COLORREF cTrack,
		COLORREF cKnob
	);
	//-------------------------------
	// sound generating functions
	//-------------------------------
	virtual void Run(void);
	//-------------------------------
	// drawing functions
	//-------------------------------
	virtual void Draw(CDC * pDC, CPoint Off, int Mode);
	virtual int CheckSelected(CPoint p, CSynthObject ** ppSelList = 0, int index = 0, int n = 0);
	BOOL CheckKnob(CPoint p);
	BOOL CheckTrack(CPoint p);
	virtual int MouseDown(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseUp(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseMove(CWnd *pWnd, int state, CPoint pos);
};

