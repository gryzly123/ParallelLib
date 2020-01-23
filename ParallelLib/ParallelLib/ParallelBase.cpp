#include "ParallelBase.h"

pParallelBase::pParallelBase()
    : threads(nullptr)
    , actualNumThreads(0)
{ }

pParallelBase::~pParallelBase()
{
	if (bNoWait.Get()) CleanupThreads();
}

void pParallelBase::CleanupThreads()
{
    if(threads == nullptr) return; //don't clean if the object never ran!

	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}
