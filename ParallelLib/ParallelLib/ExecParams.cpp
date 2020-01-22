#include "ExecParams.h"
#include <chrono>
#include <thread>

pExecParams::pExecParams(const void* _ParentTask, const int _ThreadId, const int _NumThreads)
	: ParentTask(_ParentTask)
	, ThreadId(_ThreadId)
	, NumThreads(_NumThreads)
{

}

pExecParams::~pExecParams()
{

}

void pExecParams::SleepMili(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void pExecParams::SleepMicro(int ms)
{
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

void pExecParams::SleepNano(int ns)
{
	std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}

