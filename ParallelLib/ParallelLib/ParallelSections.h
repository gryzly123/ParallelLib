#pragma once
#include "ParallelBase.h"

namespace std
{
	class thread;
}

class pSections : public pParallelBase
{
public:
	
	pSections();
	
	~pSections();

	pSections& NoWait(bool _NoWait);

	pSections& ExecuteOnMaster(bool _ExecuteOnMaster);

	void Do(std::vector<std::function<void(const pExecParams)>> Funcs);

	void DoIgnoreFirst(std::vector<std::function<void(const pExecParams)>> Funcs);
};
