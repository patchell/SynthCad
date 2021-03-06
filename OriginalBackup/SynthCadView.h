
// SynthCadView.h : interface of the CSynthCadView class
//

#pragma once

#include "SynthEngine.h"
#include "SynthParameters.h"

class CSynthCadView : public CView
{
protected: // create from serialization only
	CSynthCadView();
	DECLARE_DYNCREATE(CSynthCadView)

// Attributes
public:
	CSynthCadDoc* GetDocument() const;
	CSynthEngine *m_pSynth;
	CSynthParameters *m_pParams;
// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CSynthCadView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDebugStart();
	afx_msg void OnDebugStop();
};

#ifndef _DEBUG  // debug version in SynthCadView.cpp
inline CSynthCadDoc* CSynthCadView::GetDocument() const
   { return reinterpret_cast<CSynthCadDoc*>(m_pDocument); }
#endif

