#pragma once
#include "SynthObject.h"
#include "SynthDataPath.h"
#include "SynthSlider.h"

enum {
	FMOP_EGSTATE_RELEASE,
	FMOP_EGSTATE_ATTACK,
	FMOP_EGSTATE_DECAY
};
class CSynthFmOp :public CSynthObject
{
	COLORREF m_LineColor;
	COLORREF m_FillColor;
	CSynthSlider *m_pAtackSlider;
	CSynthSlider *m_pAttackLevelSlider;
	CSynthSlider *m_pDecaySlider;
	CSynthSlider *m_pDecayLevelSlider;
	CSynthSlider *m_pReleaseSlider;
	CRect m_Outline;
	CSynthDataPath *m_pOut;	//oscillator output
	CSynthDataPath *m_pPitch;	//pitch control
	CSynthDataPath *m_pPitchMultiplier;
	CSynthDataPath *m_pPitchDirect;	//direct pitch control (linear)
	CSynthDataPath *m_pAttack;
	CSynthDataPath *m_pAttackLevel;
	CSynthDataPath *m_pDecay;
	CSynthDataPath *m_pDecayLevel;
	CSynthDataPath *m_pRelease;
	CSynthDataPath *m_pTrig;
	float m_Phase;	//phase accumulator
	float m_ZEnvelope;	//envelope state variable
	int m_EVState;		//state of enveloope generator
public:
	//------------------------------------
	// Getter/Setter methods
	//------------------------------------
	inline void SetOut(CSynthDataPath *pSD) { m_pOut = pSD; }
	inline CSynthDataPath *GetOut(void) { return m_pOut; }
	inline void SetPitch(CSynthDataPath *pSD) { m_pPitch = pSD; }
	inline CSynthDataPath *GetPitch(void) { return m_pPitch; }
	inline void SetPitchMultiplier(CSynthDataPath *pSD) { m_pPitchMultiplier = pSD; }
	inline CSynthDataPath *GetPitchMultiplier(void) { return m_pPitchMultiplier; }
	inline void SetPitchDirect(CSynthDataPath *pSD) { m_pPitchDirect = pSD; }
	inline CSynthDataPath *GetPitchDirect(void) { return m_pPitchDirect; }
	inline void SetAttack(CSynthDataPath *pSD) { m_pAttack = pSD; }
	inline CSynthDataPath *GetAttack(void) { return m_pAttack; }
	inline void SetAttackLevel(CSynthDataPath *pSD) { m_pAttackLevel = pSD; }
	inline CSynthDataPath *GetAttackLevel(void) { return m_pAttackLevel; }
	inline void SetDecay(CSynthDataPath *pSD) { m_pDecay = pSD; }
	inline CSynthDataPath *GetDecay(void) { return m_pDecay; }
	inline void SetDecayLevel(CSynthDataPath *pSD) { m_pDecayLevel = pSD; }
	inline CSynthDataPath *GetDecayLevel(void) { return m_pDecayLevel; }
	inline void SetRelease(CSynthDataPath *pSD) { m_pRelease = pSD; }
	inline CSynthDataPath *GetRelease(void) { return m_pRelease; }
	inline void SetTrig(CSynthDataPath *pSD) { m_pTrig = pSD; }
	inline CSynthDataPath *GetTrig(void) { return m_pTrig; }
public:
	CSynthFmOp(CSynthParameters *pParams, CSynthObject *pParent);
	virtual ~CSynthFmOp();
	virtual bool Create(CPoint p1);
	virtual void Run(void);
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
private:
	float FreqInc(float freq);
	float TimeToLevel(float time);
	float LevelToFC(float level);
	float LevelToTime(float Level);
};

