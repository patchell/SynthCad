#include "stdafx.h"
#include "SynthCadDefines.h"
#include "LogicAnalyzer.h"
#include "SynthCad.h"

CLogicAnalyzer::CLogicAnalyzer()
{
	m_TriggerDelay = 0;
	m_BufferDepth = 0;
	m_Index = 0;
	m_IndexCount = 0;
	m_Triggered = 0;
}


CLogicAnalyzer::~CLogicAnalyzer()
{
}


void CLogicAnalyzer::Draw(CPoint Origin, CDC * pDC)
{
	
	{
		int min = 1, max = -1;
		int i;
		for (i = 0; i < m_BufferDepth; ++i)
		{
			if (min > (int)m_psBuffer[i]) min = (int)m_psBuffer[i];
			if (max < (int)m_psBuffer[i]) max = (int)m_psBuffer[i];
		}
		float yspan = (float)m_BufferDepth;
		float scale = yspan / float(max - min)  ;
		float offset = yspan / 2;
		CRect rect(0, 500, m_BufferDepth, m_BufferDepth + 500);
		CPen pen, *oldpen;
		CBrush br, *oldbr;
		pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		oldpen = pDC->SelectObject(&pen);
		br.CreateSolidBrush(RGB(255,33,128));
		oldbr = pDC->SelectObject(&br);
		pDC->Rectangle(&rect);
		int y;
		i = 0;
		pen.DeleteObject();
		pen.CreatePen(PS_SOLID, 2, RGB(0, 128, 64));
		y = ((int)(float(m_psBuffer[i])) * scale) + m_BufferDepth / 2 + 500;
		pDC->MoveTo(CPoint(0,y));
		for (i = 1; i < (m_BufferDepth); ++i)
		{
			y = ((int)(float(m_psBuffer[i])) * scale) + m_BufferDepth / 2 + 500;

			pDC->LineTo(CPoint(i,y)) ;
		}

		pDC->SelectObject(oldpen);
		pDC->SelectObject(oldbr);

	}
}


int CLogicAnalyzer::AddPoint(short int data)
{
	if (m_Triggered)
	{
		m_psBuffer[m_Index++] = data;
		if (m_Index == m_BufferDepth)
		{
			m_Triggered = 0;
			m_pWnd->PostMessageW(WM_DISPLAYSCOPE);
		}
	}
	else
	{
		++m_IndexCount;
		if (m_LastData < 0 && data >=0 && m_IndexCount >= m_TriggerDelay)
		{
			m_IndexCount = 0;
			m_Index = 0;
			m_Triggered = 1;
		}
		m_LastData = data;
	}
	return 0;
}
