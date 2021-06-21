// SelectMidi.cpp : implementation file
//

#include "stdafx.h"
#include "SynthCad.h"
#include "SelectMidi.h"
#include <mmsystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSelectMidi dialog


CSelectMidi::CSelectMidi(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectMidi::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectMidi)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	MidiInIndex = -1;
	MidiOutIndex = -1;
}


void CSelectMidi::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectMidi)
	DDX_Control(pDX, IDC_COMBO_MIDI_OUT_SELECT, m_ComboBox_MidiOut);
	DDX_Control(pDX, IDC_COMBO_MIDI_IN_SELECT, m_ComboBox_MidiIn);
	DDX_Control(pDX, IDC_STATIC_DISP, m_StaticDisp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectMidi, CDialog)
	//{{AFX_MSG_MAP(CSelectMidi)
	ON_CBN_SELCHANGE(IDC_COMBO_MIDI_IN_SELECT, OnSelchangeComboMidiInSelect)
	ON_CBN_SELCHANGE(IDC_COMBO_MIDI_OUT_SELECT, OnSelchangeComboMidiOutSelect)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CSelectMidi::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectMidi message handlers

BOOL CSelectMidi::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// First, find out what midi devices are availiable
	UINT nMidiInDevs = midiInGetNumDevs();
	UINT nMidiOutDevs = midiOutGetNumDevs();
	MIDIINCAPS MidiIn;
	MIDIOUTCAPS MidiOut;

	UINT i,l=0;
	for(i=0;i<nMidiInDevs;++i)
	{
		midiInGetDevCaps(i,&MidiIn,sizeof(MIDIINCAPS));
		m_ComboBox_MidiIn.InsertString(i,MidiIn.szPname);
	}
	m_ComboBox_MidiIn.SetCurSel(0);
	for(i=0;i<nMidiOutDevs;++i)
	{
		midiOutGetDevCaps(i,&MidiOut,sizeof(MIDIOUTCAPS));
		m_ComboBox_MidiOut.InsertString(i,MidiOut.szPname);
	}
	m_ComboBox_MidiOut.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectMidi::OnSelchangeComboMidiInSelect() 
{
	// TODO: Add your control notification handler code here
	MidiInIndex = m_ComboBox_MidiIn.GetCurSel();	
}

void CSelectMidi::OnSelchangeComboMidiOutSelect() 
{
	// TODO: Add your control notification handler code here
	MidiOutIndex = m_ComboBox_MidiOut.GetCurSel();	
}


void CSelectMidi::OnBnClickedOk()
{
	MidiOutIndex = m_ComboBox_MidiOut.GetCurSel();
	MidiInIndex = m_ComboBox_MidiIn.GetCurSel();
	CDialog::OnOK();
}
