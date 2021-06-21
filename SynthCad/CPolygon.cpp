#include "stdafx.h"
#include "CPolygon.h"


CPolygon::CPolygon()
{
	m_pPoints = 0;
	m_nSides = 0;
	m_MaxX = m_MaxY = 0;
	m_MinX = m_MinY = 100000;
	m_Line = 0;
	m_Fill = 0;
}

void CPolygon::Create(int nSides, COLORREF linecolor, COLORREF fillColor,CPoint *pPoints)
{
	m_pPoints = new CPoint[nSides];
	m_nSides = nSides;
	int i;
	for (i = 0; i < nSides; ++i)
	{
		m_pPoints[i] = pPoints[i];
		if (m_pPoints[i].x > m_MaxX) m_MaxX = m_pPoints[i].x;
		if (m_pPoints[i].x < m_MinX) m_MinX = m_pPoints[i].x;
		if (m_pPoints[i].y > m_MaxY) m_MaxY = m_pPoints[i].y;
		if (m_pPoints[i].y < m_MinY) m_MinY = m_pPoints[i].y;
	}
	m_Line = linecolor;
	m_Fill = fillColor;
}

CPolygon::~CPolygon()
{
	if (m_pPoints) delete [] m_pPoints;
}


void CPolygon::Draw(CDC * pDC, CPoint Offset, int mode)
{
	CPen pen, *oldPen;
	CBrush br, *oldBrush;

	CPoint *pTP = new CPoint[m_nSides];
	int i;
	for (i = 0; i < m_nSides; ++i)
		pTP[i] = m_pPoints[i] + Offset;
	pen.CreatePen(PS_SOLID, 1, m_Line);
	br.CreateSolidBrush(m_Fill);
	oldPen = pDC->SelectObject(&pen);
	oldBrush = pDC->SelectObject(&br);
	pDC->Polygon(pTP, m_nSides);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
	delete[] pTP;
}

BOOL CPolygon::PtInPolygon(CPoint point)
{
	/*****************************************
	** PtInPolygon
	**	This function determines if a point
	** is enclosed within a polygon.
	**
	** parameters:
	**	point....point to test
	** Returns: TRUE if point inside
	**          FALSE if point is outside
	*****************************************/
	int   i, j = m_nSides - 1;
	BOOL  Enclosed = 0;
	int Xintercept;
	//--------------------------------------------
	// Do a precheck agains the rectangle that
	// encloses the polygon
	//--------------------------------------------
	if (m_MinX < point.x && m_MaxX > point.x && m_MinY < point.y && m_MaxY > point.y)
	{
		//----------------------------------
		// very good chance now that point
		// is in polygon, so make a
		// thorough check
		//---------------------------------
		for (i = 0; i<m_nSides; i++)
		{
			if (((m_pPoints[i].y< point.y && m_pPoints[j].y >= point.y)
				|| (m_pPoints[j].y< point.y && m_pPoints[i].y >= point.y))
				&& (m_pPoints[i].x <= point.x || m_pPoints[j].x <= point.x))
			{
				Xintercept = m_pPoints[i].x + ((point.y - m_pPoints[i].y)*(m_pPoints[j].x - m_pPoints[i].x)) / (m_pPoints[j].y - m_pPoints[i].y);
				Enclosed ^= Xintercept < point.x;
			}
			j = i;
		}
	}
	return Enclosed;

}

