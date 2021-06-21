#if !defined(AFX_DISPLAYSTATIC_H__064F192C_EF25_4821_896B_807C8F297650__INCLUDED_)
#define AFX_DISPLAYSTATIC_H__064F192C_EF25_4821_896B_807C8F297650__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisplayStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisplayStatic window

class CStaticDisplayW : public CStatic
{
// Construction
public:
	CStaticDisplayW();
	virtual ~CStaticDisplayW();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	void InitStringFifo(int n);
	virtual void SetWindowTextW(LPCTSTR lpszString);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDisplayStatic)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	int GetTotalSize(void);
	int GenDisplayString(CString &DispString);
	int AddString(LPCWSTR s);
	int m_nStrings;
	int m_FifoTail;
	int m_FifoHead;
	int m_FifoSize;
	CString *m_FifoStrings;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYSTATIC_H__064F192C_EF25_4821_896B_807C8F297650__INCLUDED_)
