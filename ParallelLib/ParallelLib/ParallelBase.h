#pragma once
#include "SetOnce.h"
#include "ExecParams.h"
#include <functional>
#include <vector>
#include <thread>

class pParallelBase
{
protected:

	pParallelBase();

	pSetOnce<bool> bNoWait;
	pSetOnce<bool> bExecuteOnMaster;
	std::thread** threads;
	int actualNumThreads;

	void CleanupThreads();

public:

	virtual ~pParallelBase();
};

