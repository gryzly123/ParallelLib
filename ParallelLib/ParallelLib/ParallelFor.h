#pragma once
#include "Singleton.h"
#include "ParallelBase.h"
#include <mutex>

typedef std::function<void(const pExecParams, const int)> ForFunc;

namespace std
{
	class thread;
}

enum class pSchedule : unsigned char
{
	Static = 0,
	Dynamic = 1,
	Guided = 2
};

class pForChunk
{
private:
	//these should be const for more proper OOP approach,
	//but we keep a single instance of ForChunk per thread
	//for better performance (no memory allocation) and thus
	//we use an empty constructor and Init function for "creating" new chunks
	int begin;
	int end;
	int increment;

public:
	pForChunk();
	void Init(const int& Begin, const int& End, const int& Increment);
	void Do(const pExecParams& Params, ForFunc& Func);
};

class pForChunkDispenser
{
protected:
	//input parameters
	const int init;
	const int target;
	const int increment;

	//chunk generatiaon
	std::mutex chunkGetter;
	int currentBegin;
	int itersPerChunk;

public:
	pForChunkDispenser(const int& Init, const int& Target, const int& Increment/*, const int& ChunkSize*/);
	virtual ~pForChunkDispenser() { }

	virtual bool GetNextChunk(pForChunk& NextChunk) = 0;
};

class pForChunkDispenserStatic : public pForChunkDispenser
{
private:
	int numStaticChunks;
	int leftoverIters;
public:
	pForChunkDispenserStatic(const int& Init, const int& Target, const int& Increment, const int& NumThreads);
	virtual bool GetNextChunk(pForChunk& NextChunk) override;
};

class pForChunkDispenserDynamic : public pForChunkDispenser
{
private:
	int numDynamicChunks;
public:
	pForChunkDispenserDynamic(const int& Init, const int& Target, const int& Increment, const int& NumIters);
	virtual bool GetNextChunk(pForChunk& NextChunk) override;
};

class pFor : public pParallelBase
{
private:
	pSetOnce<int> numThreads;
	pSetOnce<int> chunkSize;
	pSetOnce<pSchedule> schedule;

	pForChunkDispenser* Data;

public:

	pFor();

	~pFor();

	pFor& NumThreads(int _NumThreads);

	pFor& NoWait(bool _NoWait);

	pFor& ExecuteOnMaster(bool _ExecuteOnMaster);

	pFor& ChunkSize(int _ChunkSize);

	pFor& Schedule(pSchedule _Schedule);

	void Do(const int Init, const int Target, const int Increment, ForFunc Function);

private:
	static void BeginExecuteChunks(pExecParams Params, ForFunc Function);
};
