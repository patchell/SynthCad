// GlobalLock.h: interface for the GlobalLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALLOCK_H__8349853F_82E6_4EBB_BC43_97685831BEDC__INCLUDED_)
#define AFX_GLOBALLOCK_H__8349853F_82E6_4EBB_BC43_97685831BEDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

// Basic lock class. The use of the prefix CDM was a semi-random selection.
// You could easily replace CDM with something easier to remember.
class CDMLock  
{
public:
	CDMLock(const CString & name);
	virtual ~CDMLock();
	bool Unlock();
	bool Lock(DWORD timeout = INFINITE);
   
private:
	HANDLE mutex;
};

#endif // !defined(AFX_GLOBALLOCK_H__8349853F_82E6_4EBB_BC43_97685831BEDC__INCLUDED_)
