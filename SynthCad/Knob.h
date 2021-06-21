#pragma once
#include "CPolygon.h"

class CKnob :	public CPolygon
{
public:
	CKnob();
	virtual ~CKnob();
	int GetHeight();
	void Create(int objWidth, int objHeight, COLORREF c);
	BOOL ContainsPoint(CPoint);
};

