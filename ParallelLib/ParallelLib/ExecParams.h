#pragma once

#define MASTER_TASK -1

class pExecParams
{
public:
	const void* ParentTask;
	const int ThreadId;
	const int NumThreads;

	pExecParams(const void* _ParentTask, const int _ThreadId, const int _NumThreads);
	~pExecParams();

	static void SleepMili(int ms);
	static void SleepMicro(int ms);
	static void SleepNano(int ns);
};
