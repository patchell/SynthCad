#include "stdafx.h"
#include "Lexer.h"
#include "SynthCadDefines.h"
#include <strsafe.h>

KEYWORD KeyWords[] =
{
	{TOKEN_VCO,_T("VCO")},
	{TOKEN_NONE,NULL}
};

CString TokeToString(int Token)
{
	CString csString;
	int i;
	int loop = 1;
	if (0 > Token)
	{
		csString.Format(_T("%d"), Token);
	}
	else if (256 > Token)
	{
		if (isprint(Token))
		{
			csString.Format(_T("%c"), Token);
		}
		else
		{
			csString.Format(_T("<na>%d"), Token);
		}
	}
	else
	{
		for (i = 0; KeyWords[i].KW && loop; ++i)
		{
			if (Token == KeyWords[i].Token)
			{
				csString.Format(_T("%s"), KeyWords[i].KW);
				loop = 0;
			}
		}
	}
	return csString;
}

CLexer::CLexer()
{
	m_pIN = 0;
	m_UnGetBuff = 0;
	m_Col = 0;
	m_Line = 1;
}

CLexer::CLexer(LPCWSTR path)
{
	m_Col = 0;
	m_Line = 1;
	m_UnGetBuff = 0;


	_wfopen_s(&m_pIN, path, L"r");
	if (m_pIN == NULL)
		::MessageBoxW(NULL, path, _T("ERROR:Can\'t Open"), MB_ICONHAND | MB_OK);
}

CLexer::CLexer(FILE *pIn)
{
	m_UnGetBuff = 0;
	m_pIN = pIn;
	m_Col = 0;
	m_Line = 1;
}

CLexer::~CLexer()
{
}

int CLexer::GetChar()
{
	int rV;

	if (m_UnGetBuff)
	{
		rV = m_UnGetBuff;
		m_UnGetBuff = 0;
	}
	else
	{
		rV = fgetc(m_pIN);
	}
	++m_Col;
	return rV;
}

int CLexer::UnGetChar(int c)
{
	m_UnGetBuff = c;
	if(m_Col)--m_Col;
	return 0;
}

CString CLexer::TokenToString(int Token)
{
	return TokeToString(Token);
}


int CLexer::LookUp(LPCWSTR pKW)
{
	int rV = -1;
	int i, loop;
	CString kw;
	kw.SetString(pKW);
	for (i = 0, loop = 1; KeyWords[i].KW && loop; ++i)
	{
		
		if (kw.Compare(KeyWords[i].KW) == 0)
		{
			loop = 0;
			rV = KeyWords[i].Token;
		}
	}
	return rV;
}

int CLexer::Accept(int LookaHead, int Token)
{
	/********************************************
	** Accept
	**
	** this function is used to match
	** the expected token with the current
	** token.  If they match, get the
	** next token.
	**
	** parameter:
	**	LookAHead...Look Ahead Token
	**	Token..expected token
	**	pDoc....pointer to document instance
	**
	** Returns:Next token, or -1 on error
	********************************************/
	int rV;

	if (Expect(LookaHead, Token))
		rV = Lex();
	else
		rV = -1;
	return rV;
}


int CLexer::Expect(int Lookahead, int Token)
{
	int rV = 0;

	if (Token == Lookahead)
		rV = 1;
	else
	{
		CString s;
		s.Format(_T("Expected %s  Got %s"), TokenToString(Token), TokenToString(Lookahead));
		Error(s);
	}
	return rV;
}

int CLexer::Lex()
{
	int loop, rV;
	int c;
	int token;
	loop = 1;
	while (loop)
	{
		c = GetChar();
		switch (c)
		{
			case EOF:
				loop = 0;
				rV = c;
				break;
			case ' ': case '\t':	//whitespace
				break;
			case '\n':	//end of line
				++m_Line;
				m_Col = 0;
				break;
			case '\"':	//string
				m_LexIndex = 0;
				while ((c = GetChar()) != '\"')
				{
					m_LexBuff[m_LexIndex++] = c;
				}
				m_LexBuff[m_LexIndex] = 0;
				rV = TOKEN_STRING;
				loop = 0;
				break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			case '8': case '9':	case '-': //deccimal number
				m_LexIndex = 0;
				do
				{
					m_LexBuff[m_LexIndex++] = c;
				} while (isdigit(c = GetChar()));
				UnGetChar(c);
				m_LexBuff[m_LexIndex] = 0;
				loop = 0;
				rV = TOKEN_NUM;
				break;
			case ',': case '(': case ')': case '{': case '}':
			case '[': case ']':
				rV = c;
				loop = 0;
				break;
			default:	//keywords
				m_LexIndex = 0;
				m_LexBuff[m_LexIndex++] = c;
				while (isalnum(c = GetChar()))
				{
					m_LexBuff[m_LexIndex++] = c;
				}
				UnGetChar(c);
				m_LexBuff[m_LexIndex] = 0;
				token = LookUp(m_LexBuff);
				if (token >= 0)
					rV = token;
				else
					rV = -1;
				loop = 0;
				break;
		}
	}
	return rV;
}

//--------------------------------------------------
// Misc Parsing Functions that come in handy
//--------------------------------------------------

int CLexer::Point(int Token, CPoint &p, int LookAheadToken)
{
	int x, y;

	LookAheadToken = Accept(Token, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept('(', LookAheadToken);
	if (LookAheadToken >= 0)x = _wtoi(m_LexBuff);
	if (LookAheadToken >= 0)LookAheadToken = Accept(TOKEN_NUM, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept(',', LookAheadToken);
	if (LookAheadToken >= 0)y = _wtoi(m_LexBuff);
	if (LookAheadToken >= 0)LookAheadToken = Accept(TOKEN_NUM, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept(')', LookAheadToken);
	if (LookAheadToken >= 0)p = CPoint(x, y);
	return LookAheadToken;
}

int CLexer::Color(int token,COLORREF &c, int LookaheadToken)
{
	int Red=0, Blue=0, Green=0;
	int loop = 1;

	LookaheadToken = Accept(token, LookaheadToken);
	if (LookaheadToken >= 0)LookaheadToken = Accept('(', LookaheadToken);
	while (loop)
	{
		switch (LookaheadToken)
		{
			case TOKEN_RED:
				LookaheadToken = ReadDecimalValue(TOKEN_RED, Red, LookaheadToken);
				break;
			case TOKEN_GREEN:
				LookaheadToken = ReadDecimalValue(TOKEN_GREEN, Green, LookaheadToken);
				break;
			case TOKEN_BLUE:
				LookaheadToken = this->ReadDecimalValue(TOKEN_BLUE, Blue, LookaheadToken);
				break;
			case ',':
				LookaheadToken = Accept(',', LookaheadToken);
				break;
			case ')':
				loop = 0;
				LookaheadToken = Accept(LookaheadToken, ')');
				break;
			default:
				loop = 0;
				UnexpectedToken(LookaheadToken);
				LookaheadToken = -1;	//error token
				break;
		}

	}
	if (LookaheadToken >= 0)c = RGB(Red, Green, Blue);
	return LookaheadToken;
}

int CLexer::Size(int Token,CSize &s, int LookAheadToken)
{
	int x, y;

	LookAheadToken = Accept(Token, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept('(', LookAheadToken);
	if (LookAheadToken >= 0)x = _wtoi(m_LexBuff);
	if (LookAheadToken >= 0)LookAheadToken = Accept(TOKEN_NUM, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept(',', LookAheadToken);
	if (LookAheadToken >= 0)y = _wtoi(m_LexBuff);
	if (LookAheadToken >= 0)LookAheadToken = Accept(TOKEN_NUM, LookAheadToken);
	if (LookAheadToken >= 0)LookAheadToken = Accept(')', LookAheadToken);
	if (LookAheadToken >= 0)s = CSize(x, y);
	return LookAheadToken;
}

int CLexer::ReadDecimalValue(int typetoken, int &v, int Token)
{
	Token = Accept(typetoken, Token);
	if (Token >= 0)Token = Accept('(', Token);
	if (Token >= 0)v = _wtoi(m_LexBuff);
	if (Token >= 0)Token = Accept(TOKEN_NUM, Token);
	if (Token >= 0)Token = Accept(')', Token);
	return Token;
}

int CLexer::String(int typeToken, CString &s, int Token)
{
	if (typeToken)
	{
		Token = Accept(Token, typeToken);
		Token = Accept(Token, '(');
	}
	s.SetString(m_LexBuff);
	Token = Accept(Token, TOKEN_STRING);
	if(typeToken)
		Token = Accept(Token, ')');
	return Token;
}

void CLexer::UnexpectedToken(int Token)
{
	CString text;
	text.Format(_T("Unexpected Token %s\nLine %d  Col %d"), TokeToString(Token), m_Line, m_Col);
	::MessageBoxW(NULL, text, _T("Syntax Error"),MB_OK | MB_ICONHAND);
}

void CLexer::Error(LPCWSTR s)
{
	CString Str;
	Str.Format(_T("Error:Line %d Col %d\n%s"), m_Line, m_Col, s);
	::MessageBoxW(NULL, Str,_T( "Error"), MB_OK | MB_ICONHAND);
}