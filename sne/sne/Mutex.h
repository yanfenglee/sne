#pragma once

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:

	void* m_mutex;
};