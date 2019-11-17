#pragma once
#include "SetOnce.h"
#include "ExecParams.h"
#include <functional>
#include <vector>

namespace std
{
	class thread;
}

class pSections
{
private:
	pSetOnce<bool> bNoWait;
	pSetOnce<bool> bExecuteOnMaster;
	std::thread** threads;
	int actualNumThreads;

	void CleanupThreads();

public:
	
	pSections();
	
	~pSections();

	pSections& NoWait(bool _NoWait);

	pSections& ExecuteOnMaster(bool _ExecuteOnMaster);

	void Do(std::vector<std::function<void(const pExecParams)>> Funcs);

	void DoIgnoreFirst(std::vector<std::function<void(const pExecParams)>> Funcs);
};
