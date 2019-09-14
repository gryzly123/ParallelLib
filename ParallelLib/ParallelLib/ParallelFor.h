#pragma once
#include "Singleton.h"
#include "Schedule.h"
#include "SetOnce.h"
#include <functional>

namespace std
{
	class thread;
}

class pForChunkDispenser
{
	const int init;
	const int target;
	const int increment;
	//const int chunkSize;
	std::function<void(int)> func;

public:
	pForChunkDispenser(const int& Init, const int& Target, const int& Increment/*, const int& ChunkSize*/);
	virtual pForChunk* GetNextChunk();
};

class pForChunkDispenserStatic : public pForChunkDispenser
{
private:
	int numStaticChunks;

public:
	pForChunkDispenserStatic(const int& Init, const int& Target, const int& Increment, const int& NumThreads);
	pForChunk* staticChunks;
};

class pForChunk
{
private:
	friend class pForChunkDispenser; //only ChunkDispensers can create Chunks
	pForChunk();

protected:
	const pForChunkDispenser& data;

public:
	virtual void Do() = 0;
};

class pForChunkStaticSize : public pForChunk
{
	const int chunkBegin;
	const int chunkEnd;

public:
	pForChunkStaticSize(const pForChunkDispenser& Data, const int ChunkBegin, const int ChunkEnd);

	void Do() override;
};

class pFor
{
private:
	pSetOnce<int> numThreads;
	pSetOnce<int> chunkSize;
	pSetOnce<bool> bNoWait;
	pSetOnce<bool> bExecuteOnMaster;
	pSetOnce<pSchedule> schedule;
	pForChunkDispenser* Data;
	std::thread** threads;
	int actualNumThreads;

public:

	pFor();

	~pFor();

	pFor& NumThreads(int _NumThreads);

	pFor& NoWait(bool _NoWait);

	pFor& ExecuteOnMaster(bool _ExecuteOnMaster);

	pFor& ChunkSize(bool _ChunkSize);

	void Do(const int Init, const int Target, const int Increment, const std::function<void(int)>& Function);

	void CleanupThreads();
};
