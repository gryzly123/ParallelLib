#include "ParallelFor.h"
#include <functional>
#include <thread>

pForChunkData::pForChunkData(const int& Init, const int& Target, const int& Increment, const int& ChunkSize)
	: init(Init)
	, target(Target)
	, increment(Increment)
	, chunkSize(ChunkSize)
{
	//loop packets need to have a proper size
	if (chunkSize < 1) throw;

	//increment can't be zero, because then the loop would be infinite
	if (increment == 0) throw;

	//increment MUST be positive if the loop goes from Init to Target
	//              and negative if the loop goes from Target to Init
	if ((target > init) ^ (increment > 0)) throw;
}

pForChunk::pForChunk(const pForChunkData& Data) : data(Data) { }

pForChunkStaticSize::pForChunkStaticSize(const pForChunkData& Data, const int ChunkBegin, const int ChunkEnd)
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
	Data = new pForChunkData(Init, Target, Increment, chunkSize.Get());
	
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
