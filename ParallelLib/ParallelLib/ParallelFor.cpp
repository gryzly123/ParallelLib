#include "ParallelFor.h"
#include <functional>
#include <thread>

pForChunkDispenser::pForChunkDispenser(const int& Init, const int& Target, const int& Increment)
	: init(Init)
	, target(Target)
	, increment(Increment)
	, currentBegin(Init)
{
	//increment can't be zero, because then the loop would be infinite
	if (increment == 0) throw;

	//increment MUST be positive if the loop goes from Init to Target
	//              and negative if the loop goes from Target to Init
	if ((target > init) ^ (increment > 0)) throw;
}

pForChunkDispenserStatic::pForChunkDispenserStatic(const int& Init, const int& Target, const int& Increment, const int& NumThreads)
	: pForChunkDispenser(Init, Target, Increment)
{
	int numIters = static_cast<int>(std::floor(static_cast<double>(Target - Init) / static_cast<double>(Increment)));
	numStaticChunks = numIters < NumThreads ? numIters : NumThreads;
	itersPerChunk = numIters / numStaticChunks;
	leftoverIters = numIters - (numStaticChunks * itersPerChunk);
}

bool pForChunkDispenserStatic::GetNextChunk(pForChunk& NextChunk)
{
	chunkGetter.lock();

	if (currentBegin == target)
	{
		chunkGetter.unlock();
		return false;
	}

	int currentTo = currentBegin + (increment * (itersPerChunk + leftoverIters - 1));
	if ((increment > 0 && currentTo > target) || (increment < 0 && currentTo < target))
		currentTo = target;

	NextChunk.Init(currentBegin, currentTo, increment);
	currentBegin = currentTo + increment;
	leftoverIters = 0;
	
	chunkGetter.unlock();
	return true;
}

pForChunkDispenserDynamic::pForChunkDispenserDynamic(const int& Init, const int& Target, const int& Increment, const int& NumIters)
	: pForChunkDispenser(Init, Target, Increment)
{
	itersPerChunk = NumIters;
	int totalNumIters = std::floor(static_cast<double>(Target - Init) / static_cast<double>(Increment));
	numDynamicChunks = std::floor(static_cast<double>(totalNumIters) / static_cast<double>(NumIters));
}

bool pForChunkDispenserDynamic::GetNextChunk(pForChunk& NextChunk)
{
	chunkGetter.lock();

	if ((increment > 0 && currentBegin >= target) || (increment < 0 && currentBegin <= target))
	{
		chunkGetter.unlock();
		return false;
	}

	int currentTo = currentBegin + (increment * (itersPerChunk - 1));
	if ((increment > 0 && currentTo > target) || (increment < 0 && currentTo < target))
		currentTo = target;

	NextChunk.Init(currentBegin, currentTo, increment);
	currentBegin = currentTo + increment;

	chunkGetter.unlock();
	return true;
}

pForChunk::pForChunk() { }

void pForChunk::Init(const int& Begin, const int& End, const int& Increment)
{
	begin = Begin;
	end = End;
	increment = Increment;
}

void pForChunk::Do(const pExecParams& Params, ForFunc& Func)
{
	if (increment > 0)
	{
		for (int i = begin; i <= end; i += increment)
		{
			Func(Params, i);
		}
	}
	else
	{
		for (int i = begin; i >= end; i += increment)
		{
			Func(Params, i);
		}
	}
}

pFor::pFor() { }
pFor::~pFor() { if (bNoWait.Get()) CleanupThreads(); }

pFor& pFor::NumThreads(int _NumThreads)
{
	if (_NumThreads < 1) throw;
	numThreads.Set(_NumThreads);
	return *this;
}

pFor& pFor::NoWait(bool _NoWait)
{
	bNoWait.Set(_NoWait);
	return *this;
}

pFor& pFor::ExecuteOnMaster(bool _ExecuteOnMaster)
{
	bExecuteOnMaster.Set(_ExecuteOnMaster);
	return *this;
}

pFor& pFor::ChunkSize(int _ChunkSize)
{
	chunkSize.Set(_ChunkSize);
	return *this;
}

pFor & pFor::Schedule(pSchedule _Schedule)
{
	schedule.Set(_Schedule);
	return *this;
}

void pFor::Do(const int Init, const int Target, const int Increment, ForFunc Function)
{
	//class defaults
	numThreads.OptionalSet(pSingleton::Get().NumThreads);
	bNoWait.OptionalSet(false);
	bExecuteOnMaster.OptionalSet(true);
	chunkSize.OptionalSet(1);
	schedule.OptionalSet(pSchedule::Static);

	//dynamic params
	actualNumThreads = numThreads.Get() - (bExecuteOnMaster.Get() ? 1 : 0);
	
	switch(schedule.Get())
	{
	case pSchedule::Static:
		Data = new pForChunkDispenserStatic(Init, Target, Increment, numThreads.Get());
		break;

	case pSchedule::Dynamic:
		Data = new pForChunkDispenserDynamic(Init, Target, Increment, chunkSize.Get());
		break;

	case pSchedule::Guided:
		throw; //not implemented
		break;
	}

	threads = new std::thread*[actualNumThreads];

	//execution
	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i] = new std::thread(pFor::BeginExecuteChunks, pExecParams(this, i), Function);
	}
	if (bExecuteOnMaster.Get()) BeginExecuteChunks(pExecParams(this, MASTER_TASK), Function);

	//join
	if (!bNoWait.Get())
	{
		CleanupThreads();
	}
}

void pFor::BeginExecuteChunks(pExecParams Params, ForFunc Func)
{
	pForChunkDispenser* data = ((pFor*)Params.ParentTask)->Data;
	const bool bIsStatic = ((pFor*)Params.ParentTask)->schedule.Get() == pSchedule::Static;

	pForChunk currentChunk;
	while (data->GetNextChunk(currentChunk))
	{
		currentChunk.Do(Params, Func);
		if (bIsStatic) return; //ensure that one thread doesn't get two static chunks
	}
}

void pFor::CleanupThreads()
{
	for (int i = 0; i < actualNumThreads; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}
	delete[] threads;
}
