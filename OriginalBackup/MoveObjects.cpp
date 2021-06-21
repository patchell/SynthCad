// MoveObjects.cpp: implementation of the CMoveObjects class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SynthCadDefines.h"
#include "SynthCad.h"
#include "SynthObject.h"
#include "MoveObjects.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoveObjects::CMoveObjects()
{
	m_pHead = 0;
	m_pTail = 0;
	m_Ref = CPoint(0, 0);
	m_nObjs = 0;
}

CMoveObjects::~CMoveObjects()
{

}

void CMoveObjects::AddObject(CSynthObject *pObj)
{
	if(m_pHead)
	{
		///--------------------------
		///add object to end of list
		///--------------------------
		m_pTail->SetNext(pObj);
		pObj->SetPrev(m_pTail);
		m_pTail = pObj;
	}
	else	///first object
	{
		m_pHead = pObj;
		m_pTail = pObj;
	}
	m_nObjs++;
}

CSynthObject * CMoveObjects::RemoveObject(CSynthObject *pLO)
{
	///------------------------------------------
	/// RemoveObject
	///			This function removes an object
	/// from the move buffer.  This is a DELETE
	///
	///	parameters:
	///		pLO.....pointer to object to remove
	///------------------------------------------
	if(pLO == m_pHead)
	{
		m_pHead = (CSynthObject *)m_pHead->GetNext();
		if(m_pHead) m_pHead->SetPrev(0);
	}
	else if (pLO == m_pTail)
	{
		m_pTail = (CSynthObject*)m_pTail->GetPrev();
		if(m_pTail)m_pTail->SetNext(0);
	}
	else
	{
		pLO->GetPrev()->SetNext(pLO->GetNext());
		pLO->GetNext()->SetPrev(pLO->GetPrev());
	}
	pLO->SetPrev(0);
	pLO->SetNext(0);
	m_nObjs--;
	return pLO;
}

void CMoveObjects::Draw(CDC *pDC, CPoint Off)
{
	CSynthObject *pCO = m_pHead;
	while(pCO)
	{
		pCO->Draw(pDC,OBJECT_MODE_SELECTED);
		pCO = pCO->GetNext();
	}

}

void CMoveObjects::SetRef(CPoint Ref)
{
	CSynthObject *pCO = m_pHead;
	while(pCO)
	{
		pCO->AdjustReference(Ref);
		pCO = pCO->GetNext();
	}
	m_Ref = Ref;
}

void CMoveObjects::Clear(int nDeleteFlag)
{
	CSynthObject *pCO = m_pHead;
	CSynthObject *pNext;
	while(pCO && nDeleteFlag)
	{
		pNext = pCO->GetNext();
		delete pCO;
		pCO = pNext;
	}
	this->m_pHead = 0;
	this->m_pTail = 0;
	m_nObjs = 0;
}

// Copy a MoveObjects class
void CMoveObjects::Copy(CMoveObjects & Src)
{
	CSynthObject *pObj = Src.GetHead();
	while (pObj)
	{
		AddObject(pObj->CopyObject());
		pObj = pObj->GetNext();
	}
}

void CMoveObjects::Copy(CMoveObjects *pSrc)
{
	CSynthObject *pObj = pSrc->GetHead();
	while (pObj)
	{
		AddObject(pObj->CopyObject());
		pObj = pObj->GetNext();
	}
}

// Removes object from front of list
CSynthObject * CMoveObjects::RemoveFromHead()
{
	CSynthObject *pObj;

	pObj = m_pHead;
	if (pObj)
	{
		m_pHead = pObj->GetNext();
		if(m_pHead)
			m_pHead->SetPrev(0);
		pObj->SetNext(0);
	}
	return pObj;
}


CSynthObject * CMoveObjects::RemoveFromTail()
{
	CSynthObject *pObj;

	pObj = m_pTail;
	if (pObj)
	{
		m_pTail = pObj->GetPrev();
		if(m_pTail)
			m_pTail->SetNext(0);
		pObj->SetPrev(0);
	}
	return pObj;
}
