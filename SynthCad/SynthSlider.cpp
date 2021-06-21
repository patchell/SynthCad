#include "stdafx.h"
#include "SynthCadDefines.h"
#include "SynthSlider.h"
#include "SynthParameters.h"
#include "SynthCadView.h"

CSynthSlider::CSynthSlider(CSynthParameters *pParm, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_SLIDER,pParm,pParent)
{
	m_style = 0;
	m_pSliderOut = 0;
	m_MaxVal = 127;
	m_MinVal = 0;
	m_Value = 127;
	m_Dragging = 0;
}


CSynthSlider::~CSynthSlider()
{
}

bool CSynthSlider::Create(
			CPoint P1,	//defines object rect
			CPoint P2, 
			CSynthObject *pParent,
			unsigned style,
			COLORREF cBackground,
			COLORREF cTrack,
			COLORREF cKnob

)
{
	//-------------------------------------------
	// Create a slider object
	//
	//	parameters:
	//		P1........Upper Left hand corner
	//		P2........Lower right hand corner
	//		pParent...Objects parent object
	//		style:
	//				SYNTHSLIDER_HORZ => horizontal
	//				SYNTHSLIDER_VERT => Vertical
	//----------------------------------------------
	bool rv = CSynthObject::Create(P1);
	CSize Off = P1;
	m_objRect.SetRect(P1-Off, P2-Off);
	m_objRect.NormalizeRect();
	m_style = style;
	int TrackSpace;
	int TrackWSpace;

	m_Color = cBackground;
	m_KnobColor = cKnob;
	m_TrackColor = cTrack;

	if (SYNTHSLIDER_HORZ & m_style)
	{
		//find 10% of object len
		TrackSpace = m_objRect.Width() / 10;
		TrackWSpace = m_objRect.Height() * 4 / 10;

	}
	else if (SYNTHSLIDER_VERT & m_style)
	{
		TrackSpace = m_objRect.Height() / 10;
		TrackWSpace = m_objRect.Width() * 4 / 10;
		m_Track = m_objRect;
		m_Track.InflateRect(CSize(-TrackWSpace, -TrackSpace));
		//--------------------------------
		// make the knob using a polygon
		// The first point will be in the
		// lower right hand corner
		//---------------------------------
		m_Knob.Create(m_objRect.Width(),m_objRect.Height(),RGB(0,244,0));
	}

	return rv;
}

void CSynthSlider::Run()
{
}


void CSynthSlider::Draw(CDC * pDC, CPoint Off, int Mode)
{
	//-----------------------------
	// Draw Object Rectangle
	//-----------------------------
	CPen pen, *oldPen;
	CBrush br, *oldBr;
	CRect rect;

	pen.CreatePen(PS_SOLID, 1, m_Color);
	oldPen = pDC->SelectObject(&pen);
	br.CreateSolidBrush(m_Color);
	oldBr = pDC->SelectObject(&br);
	rect = m_objRect;
	rect.OffsetRect(GetReference());
	rect.OffsetRect(Off);
	pDC->Rectangle(&rect);
	//------------------------------
	// Draw Tract
	//------------------------------
	pen.DeleteObject();
	br.DeleteObject();
	pen.CreatePen(PS_SOLID, 1, m_TrackColor);
	br.CreateSolidBrush(m_TrackColor);
	pDC->SelectObject(&pen);
	pDC->SelectObject(&br);
	rect = m_Track;
	rect.OffsetRect(GetReference());
	rect.OffsetRect(Off);
	pDC->Rectangle(&rect);
	//------------------------------
	// Draw Knob
	//------------------------------
	int pos = rect.bottom - m_Knob.GetHeight()/2 - rect.top - m_Value * m_Track.Height()/(m_MaxVal-m_MinVal);
	m_Knob.Draw(pDC,GetReference() + Off + CSize(0,pos),0);
}

int CSynthSlider::Position()
{
	int curpos = m_Track.bottom - m_Knob.GetHeight() / 2 - m_Track.top - m_Value * m_Track.Height() / (m_MaxVal - m_MinVal);
	return curpos;
}

int CSynthSlider::MouseDown(CWnd *pWnd, int state,CPoint pos) 
{
	switch (state)
	{
		case DRAWSTATE_SELECT:
			//---------------------------------
			// check to see if we are selected
			//---------------------------------
			if (m_Knob.ContainsPoint(pos-GetReference() - CSize(0,Position())))
			{
				state = DRAWSTATE_DRAG_KNOB;
				m_KnobSartDragPos = pos;
				m_Dragging = 1;
			}
			break;
		default:
			break;
	}
	return state;
}
int CSynthSlider::MouseUp(CWnd *pWnd, int state, CPoint pos)
{
	m_Dragging = 0;
	switch (state)
	{
	case DRAWSTATE_SELECT:
		break;
	case DRAWSTATE_DRAG_KNOB:
		m_Dragging = 0;
		state = DRAWSTATE_SELECT;
		break;
	}
	return state;
}
int CSynthSlider::MouseMove(CWnd *pWnd, int state, CPoint pos)
{
	CSynthEngine *pSE = (CSynthEngine *)(GetParent()->GetParent());
	DWORD SEID = pSE->GetID();
	int value;
	int curpos;
	int delta;
	CSynthCadView *pW = (CSynthCadView *)pWnd;
	switch (state)
	{
		case DRAWSTATE_DRAG_KNOB:
			//calculate screen position
			if (pW->m_MouseDown && m_Dragging)
			{
				curpos = m_Track.bottom - m_Knob.GetHeight() / 2 - m_Track.top - m_Value * m_Track.Height() / (m_MaxVal - m_MinVal);
				delta = curpos - (pos - GetReference()).cy;
				curpos -= delta;
				m_Value = (m_MaxVal - m_MinVal) * (m_Track.bottom - m_Knob.GetHeight() / 2 - m_Track.top - curpos) / m_Track.Height();
				pWnd->Invalidate();
				PostThreadMessageW(SEID, m_Message, 0, m_Value);
			}
			break;
		default:
			break;
	}
	return state;
}

int CSynthSlider::CheckSelected(CPoint p, CSynthObject ** ppSelList, int index , int n )
{
	if (CheckKnob(p) || CheckTrack(p))
	{
		ppSelList[index] = this;
		index += 1;
	}
	return index; 
}

BOOL CSynthSlider::CheckKnob(CPoint p)
{
	BOOL rV = false;
	rV = m_Knob.ContainsPoint(p);
	return rV;
}

BOOL CSynthSlider::CheckTrack(CPoint p)
{
	BOOL rV = false;
	rV = m_Track.PtInRect(p);
	return rV;

}