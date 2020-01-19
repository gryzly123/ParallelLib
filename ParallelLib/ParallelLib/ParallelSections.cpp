#include "ParallelSections.h"
#include <thread>

pSections::pSections() { }

pSections::~pSections() { if (bNoWait.Get()) CleanupThreads(); }

void pSections::CleanupThreads()
{
	if (actualNumThreads == 0) return;

	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}

pSections& pSections::NoWait(bool _NoWait)
{
	bNoWait.Set(_NoWait);
	return *this;
}

pSections& pSections::ExecuteOnMaster(bool _ExecuteOnMaster)
{
	bExecuteOnMaster.Set(_ExecuteOnMaster);
	return *this;
}

void pSections::Do(std::vector<std::function<void(const pExecParams)>> Funcs)
{
	//class defaults
	bNoWait.OptionalSet(false);
	bExecuteOnMaster.OptionalSet(true);

	//dynamic params
	actualNumThreads = Funcs.size() - (bExecuteOnMaster.Get() ? 1 : 0);
	if(actualNumThreads > 0) threads = new std::thread*[actualNumThreads];

	//execution
	for (int i = 0; i < actualNumThreads; ++i)
		threads[i] = new std::thread(Funcs[i], pExecParams(this, i, Funcs.size()));
	if (bExecuteOnMaster.Get()) Funcs[actualNumThreads](pExecParams(this, MASTER_TASK, Funcs.size()));

	//join
	if (!bNoWait.Get())
	{
		CleanupThreads();
	}
}

void pSections::DoIgnoreFirst(std::vector<std::function<void(const pExecParams)>> Funcs)
{
	Funcs.erase(Funcs.begin());
	Do(Funcs);
}
