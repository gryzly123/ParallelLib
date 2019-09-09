#include "ExecParams.h"



pExecParams::pExecParams(const void* _ParentTask, const int _ThreadId)
	: ParentTask(_ParentTask)
	, ThreadId(_ThreadId)
{
}


pExecParams::~pExecParams()
{
}
