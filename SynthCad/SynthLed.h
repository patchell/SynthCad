#pragma once
#include "SynthObject.h"
#include "SynthDataPath.h"

class CSynthLed :public CSynthObject
{
	CSynthDataPath * m_pLedDataIn;
	COLORREF m_LedColor;	//color mask
	COLORREF m_LedBKcolor;	//Background color
	float M_ColorLevel;		//led brightness
public:
	//---------------------------
	// Getter/Setter functions
	//---------------------------
	inline float *GetValuePointer() {return &M_ColorLevel;}
	inline void SetDataIn(CSynthDataPath *pD) { m_pLedDataIn = pD; }
	inline CSynthDataPath *GetDataIn() { return m_pLedDataIn; }
	inline void SetLedColor(COLORREF c) { m_LedColor = c; }
	inline COLORREF GetLedColor() { return m_LedColor; }
	inline void SetBKColor(COLORREF c) { m_LedBKcolor = c; }
	inline COLORREF GetBKColor() { return m_LedBKcolor; }
	inline void SetColorLevel(int level) { M_ColorLevel = level; }
public:
	CSynthLed(CSynthParameters *pP, CSynthObject *pParent);
	virtual ~CSynthLed();
	virtual bool Create(CPoint p1);
	//-------------------------------
	// sound generating functions
	//-------------------------------
	virtual void Run(void);
	//--------------------------------
	// Drawing functions
	//--------------------------------
	virtual LPCWSTR GetTypeString(void) { return L"LED"; }
	virtual void Draw(CDC *pDC, CPoint Off, int mode);
	virtual int MouseDown(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseUp(CWnd *pWnd, int state, CPoint pos);
	virtual int MouseMove(CWnd *pWnd, int state, CPoint pos);
	virtual void Move(CPoint p);
	virtual void Save(FILE * pO, int Indent = 0, int flags = 0);
	virtual int Parse(int Token, CLexer *pLex);
	virtual int CheckSelected(CPoint p, CSynthObject ** ppSelList = 0, int index = 0, int n = 0);
	virtual CPoint GetReference();
	virtual void AdjustReference(CPoint p);
	virtual CSynthObject *CopyObject(void) { return 0; }
};

