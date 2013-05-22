#pragma once

typedef void (*lookupcallback)(const char*, void*);

class Adns
{
public:

	static Adns* New();
	static void Delelte(Adns* ptr);

	virtual void StartLookup(const char* hostname, lookupcallback cb, void* arg) = 0;
	virtual void Update() = 0;
	virtual void FinishLookup() = 0;
};