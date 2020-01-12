#include "ParallelBase.h"

pParallelBase::pParallelBase() { }

pParallelBase::~pParallelBase()
{
	if (bNoWait.Get()) CleanupThreads();
}

void pParallelBase::CleanupThreads()
{
	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}
