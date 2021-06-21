#ifndef CSYNTHOBJECT_H
#define CSYNTHOBJECT_H

#include "SynthCadDefines.h"
#include "SynthParameters.h"
#include <Windows.h>

class CSynthParameters;
class CLexer;

class CSynthObject
{
	int m_Type;
	//-------------------------------
	// variables for drawing
	//-------------------------------
	int m_Selected;
	CPoint m_P1;	//upper right hand corner of rect
	CPoint m_P2;	//lower right hand corner of rect
	int m_Dirty;
	//-------------------------------------
	// Variables to manage all CSynthObjs
	//-------------------------------------
	CSynthObject *m_pListNext;
	CSynthObject *m_pNext;	//next object in the list of children
	CSynthObject *m_pPrev;	//previous object in the list of children
	CSynthObject *m_pHead;	//head of the list of childern
	CSynthObject *m_pTail;	//tail of the list of children
	CSynthObject *m_pParent;	//the parent object, if any
	CSynthObject *m_pSelNext;	//next selected object
	CSynthParameters *m_pParams;
public:
    CSynthObject(int nType, CSynthParameters *pParams,CSynthObject *pParent);
    virtual ~CSynthObject();
    CSynthObject(const CSynthObject& other);
	static CSynthObject *FromHandle(HWND hWnd);
	virtual bool Create(CPoint m_P1);
	//-------------------------------
	// Drawing Functions
	//-------------------------------
	virtual void Draw(CDC *pDC, CPoint Off, int mode) {};
	virtual int MouseDown(CWnd *pWnd, int state, CPoint pos) { return state; }
	virtual int MouseUp(CWnd *pWnd, int state, CPoint pos) { return state; }
	virtual int MouseMove(CWnd *pWnd, int state, CPoint pos) { return state; }
	virtual void Move(CPoint p) {};
	virtual void Save(FILE * pO, int Indent = 0, int flags = 0) {};
	virtual int Parse(int Token, CLexer *pLex) { return Token; }
	virtual int CheckSelected(CPoint p, CSynthObject ** ppSelList = 0, int index = 0, int n = 0) { return 0; }
	virtual CPoint GetReference() { return m_P1; }
	virtual void AdjustReference(CPoint p) {}
	virtual CRect GetRect(void) { return CRect(m_P1, m_P2); }
	virtual void SetSelected(int s) { m_Selected = s; }
	virtual int IsSelected(void) { return m_Selected; }
	virtual void SetDirty(int d) { m_Dirty = d; }
	virtual int IsDirty(void) { return m_Dirty; }
	//-------------------------------
	// sound generating functions
	//-------------------------------
    virtual void Run(void);
	virtual LPCWSTR GetTypeString(void) { return L"None"; }
	virtual void AddObject(CSynthObject *pO);
	virtual void InsertObject(CSynthObject *pO);
	virtual void RemoveObject(CSynthObject *pO);
	//--------------------------------
	// Getter functions
	//--------------------------------

     inline int GetType() { return m_Type; }
    inline void SetType(int val) { m_Type = val; }
    inline CSynthObject* GetNext() { return m_pNext; }
    inline void SetNext(CSynthObject *val) { m_pNext = val; }
    inline CSynthObject *GetPrev() { return m_pPrev; }
    inline void SetPrev(CSynthObject *val) { m_pPrev = val; }
	inline CSynthParameters* GetParams(void){return m_pParams;}
	inline CSynthObject * GetParent(void) { return m_pParent; }
	inline CSynthObject *GetHead() { return m_pHead; }
};

#endif // CSYNTHOBJECT_H
