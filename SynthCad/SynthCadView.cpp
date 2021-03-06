
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
#include "RenderBuffer.h"

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
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CSynthCadView construction/destruction

CSynthCadView::CSynthCadView()
{
	m_pSynth = 0;
	m_MouseDown = 0;
	m_DrawState = DRAWSTATE_SELECT;
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

void CSynthCadView::OnDraw(CDC* pDC)
{
	CSynthCadDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (m_pSynth)
	{
		CSynthObject *pObj = m_pSynth->GetHead();
		while (pObj)
		{
			pObj->Draw(pDC,CPoint(), 0);
			pObj = pObj->GetNext();
		}

	}
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

	//--------------------------------------
	// Set up synth parameters
	//--------------------------------------
	m_pParams = new CSynthParameters();
	m_pParams->SetMidiCh(0);
	m_pParams->SetButtonChan(9);
	//-------------------------------------
	// create the synthesizer engine
	//------------------------------------
	m_pSynth = new CSynthEngine(m_pParams,NULL);
	m_pSynth->Create(Dlg.m_pDeviceToUse);
	m_pSynth->m_pParentWindow = this;
	
	m_pParams->SetSamplesPerBlock(m_pSynth->m_pAudioOut->BufferSizePerPeriod());
	m_pParams->SetSampleRate(m_pSynth->m_pAudioOut->SamplesPerSecond());
	UINT32 renderBufferSizeInBytes = (m_pSynth->m_pAudioOut->BufferSizePerPeriod()  * m_pSynth->m_pAudioOut->FrameSize());
	int i;
	for (i = 0; i < 4; i += 1)
	{
		RenderBuffer *renderBuffer = new  RenderBuffer;
		if (renderBuffer == NULL)
		{
			printf("Unable to allocate render buffer\n");
			return;
		}
		renderBuffer->m_nBufferLength = renderBufferSizeInBytes;
		renderBuffer->m_pBuffer = new  BYTE[renderBufferSizeInBytes];
		if (renderBuffer->m_pBuffer == NULL)
		{
			printf("Unable to allocate render buffer\n");
			return;
		}
		//
		//  Generate tone data in the buffer.
		//
		switch (m_pSynth->m_pAudioOut->SampleType())
		{
		case CSynthCoreAudioSharedOut::SampleTypeFloat:
			m_pSynth->GenerateSamples(renderBuffer->m_pBuffer, m_pSynth->m_pAudioOut->BufferSizePerPeriod(), m_pSynth->m_pAudioOut->ChannelCount(), 0);
			//                   GenerateSineSamples<float>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
			//                                                renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
			break;
		case CSynthCoreAudioSharedOut::SampleType16BitPCM:
			//                   GenerateSineSamples<short>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
			//                                               renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
			break;
		}
		//
		//  Link the newly allocated and filled buffer into the queue.  
		//
		m_pSynth->m_pAudioOut->AddToRenderQueue(renderBuffer);
	}	//end of for loop
	m_pSynth->m_pAudioOut->m_pSE = m_pSynth;
	//
	//  The renderer takes ownership of the render queue - it will free the items in the queue when it renders them.
	//
	if (!m_pSynth->m_pAudioOut->Start(m_pSynth->m_pAudioOut->m_pRenderQueue))
	{
		m_pSynth->m_pAudioOut->Shutdown();
		printf("Could Not Start\n");
	}
	Invalidate();
}


void CSynthCadView::OnDebugStop()
{
	m_pSynth->Stop();
}


void CSynthCadView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CSelectAudioOut Dlg;

	Dlg.DoModal();

	//--------------------------------------
	// Set up synth parameters
	//--------------------------------------
	m_pParams = new CSynthParameters();
	m_pParams->SetMidiCh(0);
	m_pParams->SetButtonChan(9);
	//-------------------------------------
	// create the synthesizer engine
	//------------------------------------
	m_pSynth = new CSynthEngine(m_pParams, NULL);
	m_pSynth->Create(Dlg.m_pDeviceToUse);
	m_pSynth->m_pParentWindow = this;

	m_pParams->SetSamplesPerBlock(m_pSynth->m_pAudioOut->BufferSizePerPeriod());
	m_pParams->SetSampleRate(m_pSynth->m_pAudioOut->SamplesPerSecond());
	UINT32 renderBufferSizeInBytes = (m_pSynth->m_pAudioOut->BufferSizePerPeriod()  * m_pSynth->m_pAudioOut->FrameSize());
	int i;
	for (i = 0; i < 4; i += 1)
	{
		RenderBuffer *renderBuffer = new  RenderBuffer;
		if (renderBuffer == NULL)
		{
			printf("Unable to allocate render buffer\n");
			return;
		}
		renderBuffer->m_nBufferLength = renderBufferSizeInBytes;
		renderBuffer->m_pBuffer = new  BYTE[renderBufferSizeInBytes];
		if (renderBuffer->m_pBuffer == NULL)
		{
			printf("Unable to allocate render buffer\n");
			return;
		}
		//
		//  Generate tone data in the buffer.
		//
		switch (m_pSynth->m_pAudioOut->SampleType())
		{
		case CSynthCoreAudioSharedOut::SampleTypeFloat:
			m_pSynth->GenerateSamples(renderBuffer->m_pBuffer, m_pSynth->m_pAudioOut->BufferSizePerPeriod(), m_pSynth->m_pAudioOut->ChannelCount(), 0);
			//                   GenerateSineSamples<float>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
			//                                                renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
			break;
		case CSynthCoreAudioSharedOut::SampleType16BitPCM:
			//                   GenerateSineSamples<short>(renderBuffer->_Buffer, renderBuffer->_BufferLength, TargetFrequency,
			//                                               renderer->ChannelCount(), renderer->SamplesPerSecond(), &theta);
			break;
		}
		//
		//  Link the newly allocated and filled buffer into the queue.  
		//
		m_pSynth->m_pAudioOut->AddToRenderQueue(renderBuffer);
	}	//end of for loop
	m_pSynth->m_pAudioOut->m_pSE = m_pSynth;
	//
	//  The renderer takes ownership of the render queue - it will free the items in the queue when it renders them.
	//
	if (!m_pSynth->m_pAudioOut->Start(m_pSynth->m_pAudioOut->m_pRenderQueue))
	{
		m_pSynth->m_pAudioOut->Shutdown();
		printf("Could Not Start\n");
	}
	Invalidate();
}


void CSynthCadView::OnClose()
{
	MessageBoxW(L"Close Window");

	CView::OnClose();
}


void CSynthCadView::OnDestroy()
{
	CView::OnDestroy();

	m_pSynth->Stop();
}


void CSynthCadView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CSynthObject *pObj;
	m_MouseDown = 1;

	pObj = m_pSynth->GetHead();
	while (pObj)
	{
		m_DrawState = pObj->MouseDown(this, m_DrawState, point);
		pObj = pObj->GetNext();
	}
	CView::OnLButtonDown(nFlags, point);
}


void CSynthCadView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_MouseDown = 0;
	CSynthObject *pObj;

	pObj = m_pSynth->GetHead();
	while (pObj)
	{
		m_DrawState = pObj->MouseUp(this, m_DrawState, point);
		pObj = pObj->GetNext();
	}

	CView::OnLButtonUp(nFlags, point);
}


void CSynthCadView::OnMouseMove(UINT nFlags, CPoint point)
{
	CSynthObject *pObj;

	pObj = m_pSynth->GetHead();
	while (pObj)
	{
		m_DrawState = pObj->MouseMove(this, m_DrawState, point);
		pObj = pObj->GetNext();
	}

	CView::OnMouseMove(nFlags, point);
}
