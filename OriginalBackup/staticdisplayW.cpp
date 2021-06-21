// DisplayStatic.cpp : implementation file
//

#include "stdafx.h"
#include "StaticDisplayW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticDisplayW

CStaticDisplayW::CStaticDisplayW()
{
	m_FifoStrings = 0;
	m_FifoSize = 0;
	m_FifoHead = 0;
	m_FifoTail = 0;
	m_nStrings = 0;
}

CStaticDisplayW::~CStaticDisplayW()
{
	if(m_FifoStrings)
		delete[] m_FifoStrings;
}


BEGIN_MESSAGE_MAP(CStaticDisplayW, CStatic)
	//{{AFX_MSG_MAP(CStaticDisplayW)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticDisplayW message handlers

void CStaticDisplayW::SetWindowTextW(LPCTSTR lpszString)
{
	//---------------------------------------------------
	// This function is used to add a string to a Static
	// Text box.  The string will show up at the end of
	// the diplayed text (if any).
	//
	// parameter:
	//		lpszString....string to add to display
	//
	//---------------------------------------------------
	CString s;
	//add string to queue
	//remove oldest string (if need be)
	AddString(lpszString);
	//genereate display string
	GenDisplayString(s);
	//display the string
	CStatic::SetWindowTextW(s);
}

int CStaticDisplayW::AddString(LPCWSTR s)
{
	///-----------------------------------------
	///	AddString
	///		Adds a string to the queue that holds
	///	the strings that need to be displayed.
	///	As new strings are added, old strings
	/// are deleted.
	///
	/// parameter:
	///		s.....string to add
	/// Returns:
	///		Number of strings in queue
	///------------------------------------------------
	if(m_nStrings == m_FifoSize)	//is it full yet?
	{
		m_FifoTail++;
		if(m_FifoTail == m_FifoSize) m_FifoTail = 0;
		--m_nStrings;
	}
	m_FifoStrings[m_FifoHead++].SetString(s);
	if(m_FifoHead == m_FifoSize) m_FifoHead = 0;
	++m_nStrings;
	return m_nStrings;
}

int CStaticDisplayW::GenDisplayString(CString &DispString)
{
	//----------------------------------------------------
	// Concatanate all of the strings togheter
	// use a '\n' char between strings
	//
	// parameter:
	//	pDispString......reference to dest, must be big
	//					enough for total size of strings
	//----------------------------------------------------

	int l=0,i,t;

	t = m_FifoTail;
	for(i=0;i<m_nStrings;++i)
	{
		DispString.Append(m_FifoStrings[t]);
		DispString.AppendChar(L'\n');
		++t;
		if(t == m_nStrings) t = 0;	//wrap pointer
	}
	return m_nStrings;
}

int CStaticDisplayW::GetTotalSize()
{
	///------------------------------------------
	///	GetTotalSize
	///		Find out how many bytes one will need
	///	in order to display all the lines of text
	///
	///	Returns number of bytes required to
	/// store all text.
	///-----------------------------------------
	return 0;
}

void CStaticDisplayW::InitStringFifo(int n)
{
	///---------------------------------------------
	/// InitStringFifo
	///		Set up the string fifo for the qyeyhe
	///
	///	PARAMETER:
	///		n........number of strings in queue
	///--------------------------------------------
//	int i;
/*
	m_FifoStrings = new char *[n];
	for(i=0;i<n;++i)
		m_FifoStrings[i] = 0;
	m_FifoSize = n;
	*/
}


int CStaticDisplayW::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  Add your specialized creation code here

	return 0;
}


BOOL CStaticDisplayW::Create(LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	//--------------------------------------------
	//	Create
	//		Creates a multiline static display 
	//	text box.
	//		lpszText........Initial text to display, NULL == None
	//		dwStyle.........Style flags
	//		rect............Static Box area
	//		pParentWnd......parent
	//		nId.............Control ID
	//---------------------------------------------

//	HDC tDC;
//	tDC = ::GetDC(NULL);
//	GetTextFaceW();
//	CDC *pDC = CDC::FromHandle(tDC);
//	pDC->GetTextFaceW();
//	CDC *pDC = pParentWnd->GetDC();
//	CSize TextSize = pDC->GetTextExtent(L"Test", 4);
//	::ReleaseDC(tDC);
	int nLines = (rect.bottom - rect.top) / 20;
	m_FifoSize = nLines;
	m_FifoHead = 0;
	m_FifoTail = 0;
	m_FifoStrings = new CString[m_FifoSize];
	if (lpszText)
		AddString(lpszText);
	dwStyle |= WS_VISIBLE | BS_TEXT;
	return CStatic::Create(lpszText, dwStyle, rect, pParentWnd, nID);
}
