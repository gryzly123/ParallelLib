#pragma once
#include "ParallelBase.h"

namespace std
{
	class thread;
}

class pDo : public pParallelBase
{
private:
	pSetOnce<int> numThreads;

public:

	pDo();

	~pDo();

	pDo& NumThreads(int _NumThreads);

	pDo& NoWait(bool _NoWait);

	pDo& ExecuteOnMaster(bool _ExecuteOnMaster);

	void Do(std::function<void(const pExecParams)> Func);
};
