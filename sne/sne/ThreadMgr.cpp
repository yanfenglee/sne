#include "ThreadMgr.h"
#include <process.h>
#include <Windows.h>

ThreadMgr::ThreadMgr()
{
	m_threadexit = 0;
	unsigned int dwThreadId;
	_beginthreadex(NULL, 0, &thread_func, this, 0, &dwThreadId);
}

ThreadMgr::~ThreadMgr()
{
	m_threadexit = 1;
}

unsigned __stdcall ThreadMgr::thread_func(void* param)
{	
	ThreadMgr* pp = (ThreadMgr*)param;

	while (pp->m_threadexit == 0)
	{
		pp->m_mutex.Lock();

		for (TTasks::iterator it = pp->m_tasks.begin(); it != pp->m_tasks.end();)
		{
			ThreadTask* task = *it;

			bool finished = task->DoTask();
			if (!finished)
			{
				++it;
			}
			else
			{
				delete task;
				it = pp->m_tasks.erase(it);
			}
		}

		pp->m_mutex.Unlock();

		Sleep(1);
	}

	_endthreadex(0);

	return 0;
}

void ThreadMgr::AddTask( ThreadTask* task )
{
	m_mutex.Lock();
	m_tasks.push_back(task);
	m_mutex.Unlock();
}