#pragma once
#include "Singleton.h"
#include "Schedule.h"
#include "SetOnce.h"
#include <functional>

namespace std
{
	class thread;
}

struct pForChunkData
{
	const int init;
	const int target;
	const int increment;
	const int chunkSize;
	std::function<void(int)> func;

	pForChunkData(const int& Init, const int& Target, const int& Increment, const int& ChunkSize);
};

class pForChunk
{

protected:
	const pForChunkData& data;

	pForChunk(const pForChunkData& Data);

public:
	virtual void Do() = 0;
};

class pForChunkStaticSize : public pForChunk
{
	const int chunkBegin;
	const int chunkEnd;

public:
	pForChunkStaticSize(const pForChunkData& Data, const int ChunkBegin, const int ChunkEnd);

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
	pForChunkData* Data;
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
