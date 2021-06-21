#pragma once
#include <stdio.h>

struct KEYWORD {
	int Token;
	LPCWSTR KW;
};

class CLexer
{
	int m_UnGetBuff;
	FILE *m_pIN;
	int m_LexIndex;
	WCHAR m_LexBuff[256];
	int m_Line;
	int m_Col;
public:
	inline LPCWSTR GetLexBuff() { return m_LexBuff; }
public:
	CLexer();
	CLexer(LPCWSTR path);
	CLexer(FILE *pIn);
	virtual ~CLexer();
	int Lex();
	int GetChar();
	int UnGetChar(int c);
	int LookUp(LPCWSTR pS);
	CString TokenToString(int Token);
	int Accept(int LookaHead, int Token);
	int Expect(int LookaHead, int Token);
	void Error(LPCWSTR s);
	//-----------------------------------------------
	// parsing primintives
	//-----------------------------------------------
	int Point(int token,CPoint &p, int lookaheadtoken);
	int Color(int token,COLORREF &c, int lookaheadtoken);
	int Size(int token,CSize &s, int lookaheadtoken);
	int ReadDecimalValue(int typetoken, int &v, int Token);
	int String(int typeToken, CString &s, int Token);
	void UnexpectedToken(int Token);
};