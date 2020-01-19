#include "ParallelDo.h"
#include "Singleton.h"
#include <thread>

pDo::pDo() { }
pDo::~pDo() { if (bNoWait.Get()) CleanupThreads(); }

pDo& pDo::NumThreads(int _NumThreads)
{
	numThreads.Set(_NumThreads);
	return *this;
}

pDo& pDo::NoWait(bool _NoWait)
{
	bNoWait.Set(_NoWait);
	return *this;
}

pDo& pDo::ExecuteOnMaster(bool _ExecuteOnMaster)
{
	bExecuteOnMaster.Set(_ExecuteOnMaster);
	return *this;
}

void pDo::Do(std::function<void(const pExecParams)> Func)
{
	//class defaults
	numThreads.OptionalSet(pSingleton::Get().NumThreads);
	bNoWait.OptionalSet(false);
	bExecuteOnMaster.OptionalSet(true);

	//dynamic params
	actualNumThreads = numThreads.Get() - (bExecuteOnMaster.Get() ? 1 : 0);
	threads = new std::thread*[actualNumThreads];

	//execution
	for (int i = 0; i < actualNumThreads; ++i)
		threads[i] = new std::thread(Func, pExecParams(this, i, numThreads.Get()));
	if (bExecuteOnMaster.Get()) Func(pExecParams(this, MASTER_TASK, numThreads.Get()));

	//join
	if (!bNoWait.Get())
	{
		CleanupThreads();
	}
}

void pDo::CleanupThreads()
{
	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}
