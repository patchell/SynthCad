#ifndef CSYNTHSVFILTER_H
#define CSYNTHSVFILTER_H

#include "SynthDataPath.h"
#include "SynthObject.h"
#include "SynthSlider.h"

class CSynthSVFilter : public CSynthObject
{
	CRect m_OutLine;		//outline of module 
	COLORREF m_FillColor;
	COLORREF m_LineColor;
	CSynthSlider *m_pFreqCtl;
	CSynthSlider *m_pFreqAtnCtl;
	CSynthSlider *m_pQCtl;
	float m_Z1;
	float m_Z2;
	CSynthDataPath* m_pIn;		//filter input
	CSynthDataPath* m_pHPo;		//high pass output
	CSynthDataPath* m_pBPo;		//band pass output
	CSynthDataPath* m_pLPo;		//low pass output
	CSynthDataPath* m_pFc;		//frequency control
	CSynthDataPath* m_pQ;		//filter Q control
public:
	CSynthSVFilter(CSynthParameters *pParams, CSynthObject *pParent);
	virtual ~CSynthSVFilter();
	virtual bool Create(CPoint p1);
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


	virtual void Run();
	float Integrator(float fc, float Z, float In);
	float FreqToFc(float freq);
	inline CSynthDataPath* GetIn() { return m_pIn; }
	inline void SetIn(CSynthDataPath* val) { m_pIn = val; }
	inline CSynthDataPath* GetHPo() { return m_pHPo; }
	inline void SetHPo(CSynthDataPath* val) { m_pHPo = val; }
	inline CSynthDataPath* GetBPo() { return m_pBPo; }
	inline void SetBPo(CSynthDataPath* val) { m_pBPo = val; }
	inline CSynthDataPath* GetLPo() { return m_pLPo; }
	inline void SetLPo(CSynthDataPath* val) { m_pLPo = val; }
	inline CSynthDataPath* GetFc() { return m_pFc; }
	inline void SetFc(CSynthDataPath* val) { m_pFc = val; }
	inline CSynthDataPath* GetQ() { return m_pQ; }
	inline void SetQ(CSynthDataPath* val) { m_pQ = val; }
	inline CSynthSlider *GetFilterFreqSlider() { return m_pFreqCtl; }
	inline CSynthSlider *GetFilterQSlider() { return m_pQCtl; }
	inline CSynthSlider *GetFilterFreqAtnSlider() { return m_pFreqAtnCtl; }
};

#endif // CSYNTHSVFILTER_H
