#pragma once
#include "afxwin.h"
#include <Audioclient.h>
#include <Mmdeviceapi.h>
#include <MMDeviceAPI.h>


// CSelectAudioOut dialog

class CSelectAudioOut : public CDialog
{
	DECLARE_DYNAMIC(CSelectAudioOut)
	IMMDeviceEnumerator *deviceEnumerator;
	IMMDeviceCollection *deviceCollection;
public:
	CSelectAudioOut(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectAudioOut();
	UINT m_DevID;
	LPWSTR GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex);
	IMMDevice *m_pDeviceToUse;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIOOUTSEL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_Combo_AudioOut;
	afx_msg void OnBnClickedOk();
};
