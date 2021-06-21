#pragma once
class CPolygon
{
	int m_nSides;
	int m_MinX;
	int m_MinY;
	int m_MaxX;
	int m_MaxY;
	CPoint *m_pPoints;
	COLORREF m_Line;
	COLORREF m_Fill;
public:
	CPolygon();
	void Create(int nSides, COLORREF linecolor, COLORREF fillColor,CPoint *pPoints);
	virtual ~CPolygon();
	void Draw(CDC * pDC, int mode, CSize Offest);
	inline void SetLineColor(COLORREF c) { m_Line = c; }
	inline COLORREF GetLineColor() { return m_Line; }
	inline void SetFillColor(COLORREF c) { m_Fill = c; }
	inline COLORREF GetFillColor() { return m_Fill; }
	BOOL PtInPolygon(CPoint pt);
	CPoint *GetPoints(void) { return m_pPoints; }
//	inline COLORREF
};

