#include "ParallelFor.h"
#include <functional>
#include <thread>

pForChunkDispenser::pForChunkDispenser(const int& Init, const int& Target, const int& Increment)
	: init(Init)
	, target(Target)
	, increment(Increment)
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
	numStaticChunks = NumThreads > numIters ? NumThreads : numIters;

	staticChunks = malloc(sizeof(void*) * numStaticChunks);
	staticChunks[0].from = Init;
	staticChunks[0].to   = Init + (numIters * Increment);
	int i = 1;

	for (; i < numStaticChunks; ++i)
	{
		staticChunks[i].from = staticChunks[i - 1].to;
		staticChunks[i].to   = staticChunks[i].from + (numIters * Increment);
	}

}


pForChunk::pForChunk(const pForChunkDispenser& Data) : data(Data) { }

pForChunkStaticSize::pForChunkStaticSize(const pForChunkDispenser& Data, const int ChunkBegin, const int ChunkEnd)
	: pForChunk(Data)
	, chunkBegin(ChunkBegin)
	, chunkEnd(ChunkEnd)
{ }

void pForChunkStaticSize::Do()
{
	for (int i = chunkBegin; i < chunkEnd; i += data.increment) data.func(i);
}

pFor::pFor() { }
pFor::~pFor() { if (bNoWait.Get()) CleanupThreads(); }

pFor& pFor::NumThreads(int _NumThreads)
{
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

pFor& pFor::ChunkSize(bool _ChunkSize)
{
	chunkSize.Set(_ChunkSize);
	return *this;
}

void pFor::Do(const int Init, const int Target, const int Increment, const std::function<void(int)>& Function)
{
	//class defaults
	numThreads.OptionalSet(pSingleton::Get().NumThreads);
	bNoWait.OptionalSet(false);
	bExecuteOnMaster.OptionalSet(true);
	chunkSize.OptionalSet(1);
	schedule.OptionalSet(pSchedule::Static);

	//dynamic params
	Data = new pForChunkDispenser(Init, Target, Increment, chunkSize.Get());
	
	//switch (schedule.Get())
	//{
	//case pSchedule::Static:
	//	for (int i = 0; i < numThreads; ++i)
	//	{
	//		pForChunkStaticSize* s = new pForChunkStaticSize(chunkBegin)
	//		threads[i] = new std::thread(Func(i), )
	//	}
	//
	//case pSchedule::Dynamic:
	//
	//case pSchedule::Guided:
	//
	//}
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

