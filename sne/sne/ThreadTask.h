#pragma once

class ThreadTask
{
public:
	virtual ~ThreadTask() {}
	virtual bool DoTask() = 0;
};