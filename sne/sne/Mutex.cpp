#include "Mutex.h"
#include <windows.h>

Mutex::Mutex()
{
	m_mutex = ::CreateMutex(NULL, FALSE, NULL);
}

Mutex::~Mutex()
{
	::CloseHandle(m_mutex);
}

void Mutex::Lock()
{
	WaitForSingleObject(m_mutex, INFINITE);
}

void Mutex::Unlock()
{
	::ReleaseMutex(m_mutex);
}