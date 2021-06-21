// MoveObjects.h: interface for the CMoveObjects class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CSynthObject;
class CScale;

class CMoveObjects  
{
	CPoint m_Ref;
	int m_nObjs;		//number of objects in list
	CSynthObject * m_pHead;
	CSynthObject * m_pTail;
public:
	CMoveObjects();
	virtual ~CMoveObjects();
	void Clear(int nDeleteFlag=0);
	void SetRef(CPoint Ref);
	void Draw(CDC *pDC,CPoint Off);
	CSynthObject *GetHead(void){return m_pHead;}
	CSynthObject * RemoveObject(CSynthObject *pO);
	void AddObject(CSynthObject *pO);
	void Copy(CMoveObjects & Src);// Copy a MoveObjects class
	void Copy(CMoveObjects *pSrc);// Copy a MoveObjects class
	CSynthObject * RemoveFromHead();// Removes object from front of list
	CSynthObject * RemoveFromTail();
	inline int IsReadyToPaste(void) { return m_nObjs; }
};

