#ifndef CSYNTHEGADSR_H
#define CSYNTHEGADSR_H

#include "SynthDataPath.h"
#include "SynthObject.h"
#include "SynthSlider.h"
#include "SynthParameters.h"

enum EGStates {
	EGSTATE_RELEASE,
	EGSTATE_ATTACK,
	EGSTATE_DECAY
};

class CSynthEGadsr : public CSynthObject
{
	CRect m_Outline;
	COLORREF m_LineColor;
	COLORREF m_FillColor;
	CSynthSlider *m_pAttackCtl;
	CSynthSlider *m_pDecayCtl;
	CSynthSlider *m_pSustainCtl;
	CSynthSlider *m_pReleaseCtl;
	CSynthParameters *m_pParams;
	CSynthDataPath *m_pTrigger;
	CSynthDataPath *m_pAttack;
	CSynthDataPath *m_pDecay;
	CSynthDataPath *m_pSustain;
	CSynthDataPath *m_pRelease;
	CSynthDataPath *m_pOut;
	int m_State;
	double m_Z;
	double m_a1, m_b0;
public:
	CSynthEGadsr(CSynthParameters *Params, CSynthObject *pParent);
	virtual ~CSynthEGadsr();
	virtual void Run(void);
	virtual bool Create(CPoint p1, int Am,int Dm,int Sm,int Rm);
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

	CSynthDataPath *GetTrigger() { return m_pTrigger; }
	void SetTrigger(CSynthDataPath * val) { m_pTrigger = val; }
	CSynthDataPath *GetAttack(){return m_pAttack;}
	void SetAttack(CSynthDataPath *pd){m_pAttack = pd;}
	CSynthDataPath *GetDecay(){return m_pDecay;}
	void SetDecay(CSynthDataPath *pD){m_pDecay = pD;}
	CSynthDataPath *GetSustain(){return m_pSustain;}
	void SetSustain(CSynthDataPath *pD){m_pSustain = pD;}
    CSynthDataPath *GetRelease(){return m_pRelease;}
    void SetRelease(CSynthDataPath *pD){m_pRelease = pD;}
    CSynthDataPath *GetOut(){return m_pOut;}
    void SetOut(CSynthDataPath *pD){m_pOut = pD;}
	inline void SetFc(float Fc) {
			m_a1 = (float)exp(-twoPI * Fc);
			m_b0 = float(1.0 - m_a1);
	}
	float TimeToLevel(float time);
	float LevelToTime(float Level);
	float LevelToFC(float level);
	//-------------------------------
	// Getter Functions
	//-------------------------------
	inline COLORREF GetLineColor() { return m_LineColor; }
	inline void SetLineColor(COLORREF c) { m_LineColor = c; }
	inline COLORREF GetFillColor() { return m_FillColor; }
	inline void SetFillColor(COLORREF c) { m_FillColor = c; }
	inline CSynthSlider *GetAttackSlider() { return m_pAttackCtl; }
	inline CSynthSlider *GetDecaySlider() { return m_pDecayCtl; }
	inline CSynthSlider *GetSustainSlider() { return m_pSustainCtl; }
	inline CSynthSlider *GetReleaseSlider() { return m_pReleaseCtl; }
};

#endif // CSYNTHEGADSR_H
