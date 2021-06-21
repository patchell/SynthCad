// GlobalLock.cpp: implementation of the GlobalLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Lock.h"

#include <afxmt.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDMLock::CDMLock(const CString & name)
{
	mutex = CreateMutex(NULL, 0, name);	
}

CDMLock::~CDMLock()
{
	CloseHandle(mutex);
}

// Release lock.
bool CDMLock::Unlock()
{
	if (mutex) {
		if (ReleaseMutex(mutex))
			return true;
	}
	return false;
}

// Get lock. If timeout expires than return fail.
bool CDMLock::Lock(DWORD timeout)
{
	if (mutex) {
		if (WaitForSingleObject(mutex, timeout) == WAIT_OBJECT_0) 
			return true;
	}
	return false;
}


