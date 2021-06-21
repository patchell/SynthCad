#if !defined(AFX_SELECTMIDI_H__4C724074_173C_4DE8_A0BA_3D03155EC88F__INCLUDED_)
#define AFX_SELECTMIDI_H__4C724074_173C_4DE8_A0BA_3D03155EC88F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMidi.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectMidi dialog

class CSelectMidi : public CDialog
{
// Construction
public:
	UINT MidiOutIndex;
	UINT MidiInIndex;
	CSelectMidi(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectMidi)
	enum { IDD = IDD_DIALOG_SELECTMIDI };
	CComboBox	m_ComboBox_MidiOut;
	CComboBox	m_ComboBox_MidiIn;
	CStatic	m_StaticDisp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMidi)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectMidi)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboMidiInSelect();
	afx_msg void OnSelchangeComboMidiOutSelect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMIDI_H__4C724074_173C_4DE8_A0BA_3D03155EC88F__INCLUDED_)
