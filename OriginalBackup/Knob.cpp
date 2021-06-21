#include "stdafx.h"
#include "Knob.h"


CKnob::CKnob()
{
}


CKnob::~CKnob()
{
}


int CKnob::GetHeight()
{
	int rv =GetPoints()[1].y - GetPoints()[0].y;
	return rv;
}


void CKnob::Create(int Width,int Height,COLORREF c)
{
	int WSpace = Width * 2 / 10;
	int KnobH = Height / 10;
	CPoint pts[5] = {
		{ WSpace,KnobH },
	{ WSpace, 0 },
	{ Width - 3 * WSpace,0 },
	{ Width -  WSpace,KnobH / 2 },
	{ Width - 3 * WSpace,KnobH }
	};
	CPolygon::Create(5, c, c, pts);
}
