#ifndef POLYBLEPOSC_H
#define POLYBLEPOSC_H

#include "SynthObject.h"
#include "SynthDataPath.h"
#include "SynthSlider.h"

enum OscMode {
	PBOSC_MODE_SINE,
	PBOSC_MODE_TRI,
	PBOSC_MODE_SAW,
	PBOSC_MODE_PULSE,
};

class CSynthPolyBLEPOsc : public CSynthObject
{
	COLORREF m_LineColor;
	COLORREF m_FillColor;
	CSynthSlider *m_pPulsWidthCtl;
	CSynthSlider *m_pWaveSelCtl;
	CRect m_Outline;

	CSynthDataPath *m_pOut;	//oscillator output
	CSynthDataPath *m_pPitch;	//pitch control
	CSynthDataPath *m_pPulseWidth;	//pulse width control
	CSynthDataPath *m_pWaveSelect;
	OscMode m_Mode;	//waveform mode
	float m_Phase;	//phase accumulator
	float m_y1, m_y2, m_y3;
	float m_LIz;	//leaky integrator state variable
public:
	//-------------------------------
	// Drawing Functions
	//-------------------------------
	virtual void Draw(CDC *pDC, CPoint Off, int mode);
	virtual int MouseDown(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseUp(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseMove(CWnd *pWnd, int state, CPoint pos);
	virtual void Move(CPoint p);
	virtual void Save(FILE * pO, int Indent = 0, int flags = 0);
	virtual int Parse(int Token, CLexer *pLex);
	virtual int CheckSelected(CPoint p, CSynthObject ** ppSelList = 0, int index = 0, int n = 0);
	virtual void AdjustReference(CPoint p);
	virtual CRect GetRect(void);
	//--------------------------------------------
	// Getter functions
	//--------------------------------------------
	inline void SetOutput(CSynthDataPath *pDP){m_pOut = pDP;}
	inline CSynthDataPath *GetOutput(void){return m_pOut;}
	inline void SetPitch(CSynthDataPath *pDP){m_pPitch = pDP;}
	inline CSynthDataPath *GetPitch(void){return m_pPitch;}
	inline void SetPW(CSynthDataPath *pDP){m_pPulseWidth = pDP;}
	inline CSynthDataPath *GetPW(){return m_pPulseWidth;}
	inline void SetOscMode(OscMode m){m_Mode = m;}
	inline void SetWaveSel(CSynthDataPath *pDP) { m_pWaveSelect = pDP; }
	inline CSynthSlider *GetPwmSlider() { return m_pPulsWidthCtl; }
	//---------------------------------------------
	// Implementation
	//---------------------------------------------
	CSynthPolyBLEPOsc(CSynthParameters *pParams, CSynthObject *pParent);
	virtual ~CSynthPolyBLEPOsc();
	virtual void Run();
	virtual bool Create(CPoint p1);
private:
	//----------------------------------------------
	// Oscillator functions
	//----------------------------------------------
	float PolyBlep(float phase, float dphase);
	float Saw(float phase, float dphase);
	float Pulse(float phase,float dphase,float trip);
	float Tri(float phase, float dphase);
	float FreqInc(float freq);
};

#endif // POLYBLEPOSC_H
