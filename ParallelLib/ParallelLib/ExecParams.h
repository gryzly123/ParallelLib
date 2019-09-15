#pragma once

#define MASTER_TASK -1

#define THREAD_ID ___pExecParams.ThreadId

class pExecParams
{
public:
	const void* ParentTask;
	const int ThreadId;

	pExecParams(const void* _ParentTask, const int _ThreadId);
	~pExecParams();
};
