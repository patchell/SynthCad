#pragma once
class CLogicAnalyzer
{
public:
	int m_TriggerDelay;
	int m_BufferDepth;
	int m_IndexCount;
	int m_Index;
	int m_Triggered;
	short int *m_psBuffer;
	short int m_LastData;
	CWnd *m_pWnd;	//parent window
public:
	CLogicAnalyzer();
	virtual ~CLogicAnalyzer();
	void Draw(CPoint Origin, CDC * pDC);
	int AddPoint(short int data);
};

