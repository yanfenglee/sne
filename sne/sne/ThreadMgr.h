#pragma once
#include "ThreadTask.h"
#include "Singleton.h"
#include "Mutex.h"
#include <list>

class ThreadMgr : public Singleton<ThreadMgr>
{
public:

	void AddTask(ThreadTask* task);

	ThreadMgr();
	~ThreadMgr();

private:
	unsigned static __stdcall thread_func(void* param);

	Mutex m_mutex;
	int m_threadexit;

	typedef std::list<ThreadTask*> TTasks;

	TTasks m_tasks;
};