#pragma once
#include "SetOnce.h"
#include "ExecParams.h"
#include <functional>

namespace std
{
	class thread;
}

class pDo
{
private:
	pSetOnce<int> numThreads;
	pSetOnce<bool> bNoWait;
	pSetOnce<bool> bExecuteOnMaster;
	std::thread** threads;
	int actualNumThreads;

	void CleanupThreads();

public:

	pDo();

	~pDo();

	pDo& NumThreads(int _NumThreads);

	pDo& NoWait(bool _NoWait);

	pDo& ExecuteOnMaster(bool _ExecuteOnMaster);

	void Do(std::function<void(const pExecParams)> Func);
};
