// SelectAudioOut.cpp : implementation file
//

#include "stdafx.h"
#include "SynthCad.h"
#include "SelectAudioOut.h"
#include "afxdialogex.h"
#include <strsafe.h>
#include <Functiondiscoverykeys_devpkey.h>

//#include <mmsystem.h>


// CSelectAudioOut dialog

IMPLEMENT_DYNAMIC(CSelectAudioOut, CDialog)

CSelectAudioOut::CSelectAudioOut(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_AUDIOOUTSEL, pParent)
{
	deviceEnumerator = NULL;
	deviceCollection = NULL;
}

CSelectAudioOut::~CSelectAudioOut()
{
}

void CSelectAudioOut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SELAUDIO, m_Combo_AudioOut);
}


BEGIN_MESSAGE_MAP(CSelectAudioOut, CDialog)
	ON_BN_CLICKED(IDOK, &CSelectAudioOut::OnBnClickedOk)
END_MESSAGE_MAP()


// CSelectAudioOut message handlers


BOOL CSelectAudioOut::OnInitDialog()
{
	CDialog::OnInitDialog();

	HRESULT hr;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
	hr = deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);

	UINT deviceCount;
	hr = deviceCollection->GetCount(&deviceCount);
	if (FAILED(hr))
	{
//		printf("Unable to get device collection length: %x\n", hr);
	}
	for (UINT i = 0; i < deviceCount; i += 1)
	{
		LPWSTR deviceName;

		deviceName = GetDeviceName(deviceCollection, i);
		if (deviceName == NULL)
		{
		}
		m_Combo_AudioOut.InsertString(i, deviceName);
//		printf("    %d:  %S\n", i + 3, deviceName);
		free(deviceName);
	}
	m_Combo_AudioOut.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



void CSelectAudioOut::OnBnClickedOk()
{
	m_DevID = m_Combo_AudioOut.GetCurSel();
	deviceCollection->Item(m_DevID, &m_pDeviceToUse);
	CDialog::OnOK();
}

LPWSTR CSelectAudioOut::GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex)
{
	IMMDevice *device;
	LPWSTR deviceId;
	HRESULT hr;

	hr = DeviceCollection->Item(DeviceIndex, &device);
	if (FAILED(hr))
	{
//		printf("Unable to get device %d: %x\n", DeviceIndex, hr);
		return NULL;
	}
	hr = device->GetId(&deviceId);
	if (FAILED(hr))
	{
//		printf("Unable to get device %d id: %x\n", DeviceIndex, hr);
		return NULL;
	}

	IPropertyStore *propertyStore;
	hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
	device->Release();
	if (FAILED(hr))
	{
//		printf("Unable to open device %d property store: %x\n", DeviceIndex, hr);
		return NULL;
	}

	PROPVARIANT friendlyName;
	PropVariantInit(&friendlyName);
	hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
	if (propertyStore)
	{
		propertyStore->Release();
		propertyStore = 0;
	}

	if (FAILED(hr))
	{
//		printf("Unable to retrieve friendly name for device %d : %x\n", DeviceIndex, hr);
		return NULL;
	}

	wchar_t deviceName[128];
	hr = StringCbPrintf(deviceName, sizeof(deviceName), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId);
	if (FAILED(hr))
	{
//		printf("Unable to format friendly name for device %d : %x\n", DeviceIndex, hr);
		return NULL;
	}

	PropVariantClear(&friendlyName);
	CoTaskMemFree(deviceId);

	wchar_t *returnValue = _wcsdup(deviceName);
	if (returnValue == NULL)
	{
//		printf("Unable to allocate buffer for return\n");
		return NULL;
	}
	return returnValue;
}
