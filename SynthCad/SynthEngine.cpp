#include "stdafx.h"

#include "SynthObject.h"
#include "SynthEngine.h"
#include "SynthCoreAudioSharedOut.h"

#include <functiondiscoverykeys.h>


UINT CSynthEngine::SynthEngWorker(LPVOID pD)
{
	//------------------------------------
	// This message handler is used for
	// dispatching messages among the various
	// modules, until I can come up with a 
	// better way to do it.
	//------------------------------------
	CSynthEngine *pSE = (CSynthEngine *)pD;
	printf("Snth Engine Thread Started\n");
	int loop = 1;
	while (loop)
	{
		MSG message;

		GetMessage(&message, NULL, 0, 0);
		switch (message.message)
		{
			case MSG_KILLTHREAD:
				loop = 0;
				break;
			case WM_ADSR_ATTACK:	//Attack
				pSE->m_pA->SetData(NoteToBiLevel(message.lParam));
				pSE->m_pEnv->GetAttackSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_ADSR_DECAY:	//decay
				pSE->m_pD->SetData(NoteToBiLevel(message.lParam));
				pSE->m_pEnv->GetDecaySlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_ADSR_SUSTAIN:	//sustain
				pSE->m_pS->SetData(NoteToLevel(message.lParam));
				pSE->m_pEnv->GetSustainSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_ADSR_RELEASE:	//release
				pSE->m_pR->SetData(NoteToBiLevel(message.lParam));
				pSE->m_pEnv->GetReleaseSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_LK25_KNOB_5:	//filter Q	
				pSE->m_pQ->SetData(NoteToLevel(message.lParam));
				pSE->m_pFilt->GetFilterQSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_LK25_KNOB_6:	//filter freq
				pSE->m_pF->SetData(NoteToLevel(message.lParam));
				pSE->m_pFilt->GetFilterFreqSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_OSC_PULSEWIDTH:	//Osc PWM
				pSE->m_pPWM->SetData(NoteToLevel(message.lParam));
				pSE->m_pOsc1->GetPwmSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_LK25_KNOB_8:	//fiter ADSR atn
				pSE->m_pMixEVLevel->SetData(NoteToLevel(message.lParam));
				pSE->m_pFilt->GetFilterFreqAtnSlider()->SetValue(message.lParam);
				pSE->m_pParentWindow->Invalidate();
				break;
			case WM_NOTE_ON:
				pSE->m_pOscPitch->SetData(NoteToLevel(message.wParam));
				break;
			case WM_NOTE_OFF:
				break;
			case WM_ADSR_GATE:
				if (message.wParam)
					pSE->m_pGate->SetData((float)1.0);
				else
					pSE->m_pGate->SetData((float)0.0);
				break;
			case WM_OSC_WAVESEL:
				pSE->m_pOscWaveSel->SetData(NoteToLevel(message.wParam));
				break;
		}

	}
	return 0;
}

CSynthEngine::CSynthEngine(CSynthParameters *pParams, CSynthObject *pParent):CSynthObject(OBJECT_TYPE_ENGINE, pParams,pParent)
{
	m_pRenderQueue = NULL;
	m_ppQueueTail = &m_pRenderQueue;
}

CSynthEngine::~CSynthEngine()
{
}

void CSynthEngine::Draw(CDC *pDC, CPoint Off, int mode)
{
	CSynthObject *pObj = GetHead();
	while (pObj)
	{
		pObj->Draw(pDC, CPoint(),mode);
		pObj = pObj->GetNext();
	}
}

float CSynthEngine::RunIt()
{
	//------------------------------------
	// RunIt
	//		Generates audio output
	//------------------------------------
	float d=0.0;

	CSynthObject *pSO = GetHead();
	while (pSO)
	{
		if (OBJECT_TYPE_AUDIO == pSO->GetType())
		{
			pSO->Run();
			d = ((CSynthCoreAudioSharedOut *)pSO)->GetData();
		}
		else
			pSO->Run();
		pSO = pSO->GetNext();
	}
	return d;
}

void CSynthEngine::GenerateSamples(BYTE *buff,int nSamples,int nCh, DWORD type)
{
	//---------------------------------------
	// we need to check to see what size
	// the stream data is in
	//----------------------------------------
	if (SampleTypeFloat == m_pAudioOut->GetSampleType())
	{
		float *databuff = (float *)buff;
		float d;
		int i, index, j;

		for (i = 0, index = 0; i < nSamples; ++i)
		{
			d = RunIt();
			for (j = 0; j < nCh; ++j)
				databuff[index++] = d;
		}
	}
	else if (SampleType16BitPCM == m_pAudioOut->GetSampleType())
	{
		short *databuff = (short *)buff;
		short d;
		int i, index, j;

		for (i = 0, index = 0; i < nSamples; ++i)
		{
			d = (short)(RunIt() * 32760.0);
			for (j = 0; j < nCh; ++j)
				databuff[index++] = d;
		}

	}
}

bool CSynthEngine::Create(IMMDevice *Endpoint)
{
	Init(Endpoint);
	
	return CSynthObject::Create(CPoint());
}

void CSynthEngine::Init(IMMDevice *Endpoint)
{
	//---------------------------------
	// Create the synth here.
	//----------------------------------
	// Audio Output
	m_pAudioOut = new CSynthCoreAudioSharedOut(GetParams(), this, Endpoint,false, eMultimedia);
	m_pAudioOut->Create(CPoint());
	// Midi Input
	m_pMidiIn = new CSynthMidiIN(GetParams(), this);
	m_pMidiIn->Create( this);

	m_pPatch1 = new CSynthDataPath(GetParams(), this);
	m_pPatch1->Create(CPoint());
	AddObject(m_pPatch1);
	//VCO
	m_pOsc1 = new CSynthPolyBLEPOsc(GetParams(), this);
	m_pOsc1->Create(CPoint(300,0));
	AddObject(m_pOsc1);
	m_pOscPitch = new CSynthDataPath(GetParams(), this);
	m_pOscPitch->Create(CPoint());
	m_pOscWaveSel = new CSynthDataPath(GetParams(), this);
	m_pOscWaveSel->Create(CPoint());
	m_pPWM = new CSynthDataPath(GetParams(), this);
	//ADSR
	m_pEnv = new CSynthEGadsr(GetParams(), this);
	m_pEnv->Create(CPoint(100,0),WM_ADSR_ATTACK,WM_ADSR_DECAY,WM_ADSR_SUSTAIN,WM_ADSR_RELEASE);
	AddObject(m_pEnv);
	m_pA = new CSynthDataPath(GetParams(), this);
	m_pA->Create(CPoint());
	m_pD = new CSynthDataPath(GetParams(), this);
	m_pD->Create(CPoint());
	m_pS = new CSynthDataPath(GetParams(), this);
	m_pS->Create(CPoint());
	m_pR = new CSynthDataPath(GetParams(), this);
	m_pR->Create(CPoint());
	m_pGate = new CSynthDataPath(GetParams(), this);
	m_pGate->Create(CPoint());
	m_pEVOut = new CSynthDataPath(GetParams(), this);
	m_pEVOut->Create(CPoint());
	//Filter Fc Mixer
	m_pFiltFreqMix = new CSynthMixer(2,GetParams(), this);
	m_pFiltFreqMix->Create(CPoint());
	AddObject(m_pFiltFreqMix);
	m_pFCmix = new CSynthDataPath(GetParams(), this);
	m_pFCmix->Create(CPoint());
	m_pMixEVLevel = new CSynthDataPath(GetParams(), this);
	m_pF = new CSynthDataPath(GetParams(), this);
	m_pMixFcLevel = new CSynthDataPath(GetParams(), this);
	// VCF
	m_pFilt = new CSynthSVFilter(GetParams(), this);
	m_pFilt->Create(CPoint(400,0));
	AddObject(m_pFilt);
	m_pQ = new CSynthDataPath(GetParams(), this);
	m_pVCA = new CSynthDataPath(GetParams(), this);
	//VCA
	m_pVca = new CSynthVCA(GetParams(), this);
	m_pVca->Create(CPoint());
	m_pOutPatch = new CSynthDataPath(GetParams(), this);
	AddObject(m_pVca);
	AddObject(m_pAudioOut);

	//-----------------------------
	// Connect the objects together
	//-----------------------------
	m_pOsc1->SetOutput(m_pPatch1);
	m_pOsc1->SetPitch(m_pOscPitch);
	m_pOscPitch->SetData(FreqToLevel(440));
	m_pOsc1->SetWaveSel(m_pOscWaveSel);
	m_pOscWaveSel->SetData((float)0.8);	//sine wave
	m_pOsc1->SetPW(m_pPWM);
	//envelope generator
	m_pEnv->SetAttack(m_pA);
	m_pEnv->SetDecay(m_pD);
	m_pEnv->SetSustain(m_pS);
	m_pEnv->SetRelease(m_pR);
	m_pEnv->SetTrigger(m_pGate);
	m_pEnv->SetOut(m_pEVOut);
	//filter
	m_pFilt->SetIn(m_pPatch1);
	m_pFilt->SetFc(m_pFCmix);
	m_pFilt->SetQ(m_pQ);
	m_pFilt->SetBPo(m_pVCA);
	//mixer
	m_pFiltFreqMix->SetIn(0, m_pF);
	m_pFiltFreqMix->SetIn(1, m_pEVOut);
	m_pFiltFreqMix->SetControl(0, m_pMixFcLevel);
	m_pMixFcLevel->SetData((float)0.8);
	m_pFiltFreqMix->SetControl(1, m_pMixEVLevel);
	m_pFiltFreqMix->SetOut(m_pFCmix);
	//VCA
	m_pVca->SetIN(m_pVCA);
	m_pVca->SetControl(m_pEVOut);
	m_pVca->SetOut(m_pOutPatch);
	m_pAudioOut->SetDataInput(m_pOutPatch);
	//-----------------------------
	// Start the synth
	//-----------------------------
	BeginThread();
	m_pAudioOut->Open();
}


void CSynthEngine::Stop()
{
	this->m_pAudioOut->Close();
	m_pMidiIn->KillThead();
	TerminateThread();
}

void CSynthEngine::BeginThread()
{
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSynthEngine::SynthEngWorker, (LPVOID)this, 0, NULL);
	m_ThreadID = GetThreadId(m_hThread);
}
void CSynthEngine::TerminateThread()
{
	PostThreadMessageW(GetThreadId(m_hThread), MSG_KILLTHREAD, 0, 0);
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, INFINITE);

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

//	m_pSynthThread->PostThreadMessageW(MSG_KILLTHREAD, 0, 0);
//	Sleep(1);
//	DWORD dwExitCode;
//	do
//	{
//		::GetExitCodeThread(m_pSynthThread->m_hThread, &dwExitCode);
//		//--------------------------------------------------------
//		// in order fot the exit code to change, we need to force
//		// a context switch.  We do this by going to sleep for
//		// zero milliseconds...I personally find this to be
//		// klugey, but this is windows we are talking about here
//		//--------------------------------------------------------
//		Sleep(0L);
//	} while (dwExitCode);
//	if (dwExitCode == 0)	// Is The thread is still running.
//	{
//		delete m_pSynthThread;
//	}

}
