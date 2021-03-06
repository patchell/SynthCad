
// SynthCadView.cpp : implementation of the CSynthCadView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SynthCad.h"
#endif

#include "SynthCadDoc.h"
#include "SynthCadView.h"
#include "SelectAudioOut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSynthCadView

IMPLEMENT_DYNCREATE(CSynthCadView, CView)

BEGIN_MESSAGE_MAP(CSynthCadView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSynthCadView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_DEBUG_START, &CSynthCadView::OnDebugStart)
	ON_COMMAND(ID_DEBUG_STOP, &CSynthCadView::OnDebugStop)
END_MESSAGE_MAP()

// CSynthCadView construction/destruction

CSynthCadView::CSynthCadView()
{
	// TODO: add construction code here

}

CSynthCadView::~CSynthCadView()
{
}

BOOL CSynthCadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSynthCadView drawing

void CSynthCadView::OnDraw(CDC* /*pDC*/)
{
	CSynthCadDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CSynthCadView printing


void CSynthCadView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSynthCadView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSynthCadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSynthCadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CSynthCadView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSynthCadView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSynthCadView diagnostics

#ifdef _DEBUG
void CSynthCadView::AssertValid() const
{
	CView::AssertValid();
}

void CSynthCadView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSynthCadDoc* CSynthCadView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSynthCadDoc)));
	return (CSynthCadDoc*)m_pDocument;
}
#endif //_DEBUG


// CSynthCadView message handlers


void CSynthCadView::OnDebugStart()
{
	CSelectAudioOut Dlg;

	Dlg.DoModal();

	m_pParams = new CSynthParameters();
	m_pParams->SetSampleRate(48000);
	m_pParams->SetMidiCh(0);
	m_pParams->SetButtonChan(9);
	m_pParams->SetSamplesPerBlock(256);
	m_pSynth = new CSynthEngine(m_pParams,NULL);
	m_pSynth->Create(NULL,Dlg.m_pDeviceToUse);
	
}


void CSynthCadView::OnDebugStop()
{
	m_pSynth->Stop();
}
