#include <iostream>
#include <mutex>
#include <queue>

class pSingleton
{
private:

	static pSingleton* ptr;

	pSingleton()
		: NumThreads(std::thread::hardware_concurrency())
	{ }

public:
	const unsigned int NumThreads;

	static const pSingleton& Get()
	{
		if (ptr == nullptr) ptr = new pSingleton;
		return *ptr;
	}
};
pSingleton* pSingleton::ptr;

template<class T>
class pVariable
{
private:
	std::mutex lock;

public:
	//operator=(T Object2) override
	//{
	//	Lock.lock();
	//	Object = Object2;
	//	Lock.unlock();
	//}
};

template<class T>
class pVariableShared : pVariable<T>
{
private:
	T& object;

public:
	pVariableShared(T& Object) : object(Object) { }
	//operator=(T Object2) override
	//{
	//	Lock.lock();
	//	Object = Object2;
	//	Lock.unlock();
	//}
};


template<class T>
class pSetOnce
{
private:
	T Object;
	bool bWasSet = false;

public:
	void Set(const T& Data)
	{
		if (bWasSet) throw;
		Object = Data;
		bWasSet = true;
	}

	bool OptionalSet(const T& Data)
	{
		if (bWasSet) return false;
		Object = Data;
		bWasSet = true;
		return true;
	}

	const T& Get() { return Object; }
	bool WasSet() { return bWasSet; }
};

class pDo
{
private:
	pSetOnce<int> numThreads;
	pSetOnce<bool> bNoWait;
	pSetOnce<bool> bExecuteOnMaster;
	std::thread** threads;
	int actualNumThreads;

public:

	pDo() { }
	~pDo() { if (bNoWait.Get()) CleanupThreads(); }

	pDo& NumThreads(int _NumThreads)
	{
		numThreads.Set(_NumThreads);
		return *this;
	}

	pDo& NoWait(bool _NoWait)
	{
		bNoWait.Set(_NoWait);
		return *this;
	}

	pDo& ExecuteOnMaster(bool _ExecuteOnMaster)
	{
		bExecuteOnMaster.Set(_ExecuteOnMaster);
		return *this;
	}

	void Do(std::function<void()> Func)
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
			threads[i] = new std::thread(Func);
		if(bExecuteOnMaster.Get()) Func();

		//join
		if (!bNoWait.Get())
		{
			CleanupThreads();
		}
	}

	void CleanupThreads()
	{
		for (int i = 0; i < actualNumThreads; ++i)
		{
			threads[i]->join();
			delete threads[i];
		}
		delete[] threads;
	}
};

struct pForChunkData
{
	const int init;
	const int target;
	const int increment;
	const int chunkSize;
	std::function<void(int)> func;

	pForChunkData(const int& Init, const int& Target, const int& Increment, const int& ChunkSize)
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
		if ((target < init) ^ (increment > 0)) throw;
	}
};

class pForChunk
{
protected:
	const pForChunkData& data;
	pForChunk(const pForChunkData& Data) : data(Data) { }

public:
	virtual void Do() = 0;
};

class pForChunkStaticSize : public pForChunk
{
	const int chunkBegin;
	const int chunkEnd;

public:
	pForChunkStaticSize(const pForChunkData& Data, const int ChunkBegin, const int ChunkEnd)
		: pForChunk(Data)
		, chunkBegin(ChunkBegin)
		, chunkEnd(ChunkEnd)
	{ }

	void Do() override
	{
		for (int i = chunkBegin; i < chunkEnd; i += data.increment) data.func(i);
	}
};

enum pSchedule
{
	Static = 0,
	Dynamic = 1,
	Guided = 2
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

	pFor() { }
	~pFor() { if (bNoWait.Get()) CleanupThreads(); }

	pFor& NumThreads(int _NumThreads)
	{
		numThreads.Set(_NumThreads);
		return *this;
	}

	pFor& NoWait(bool _NoWait)
	{
		bNoWait.Set(_NoWait);
		return *this;
	}

	pFor& ExecuteOnMaster(bool _ExecuteOnMaster)
	{
		bExecuteOnMaster.Set(_ExecuteOnMaster);
		return *this;
	}

	pFor& ChunkSize(bool _ChunkSize)
	{
		chunkSize.Set(_ChunkSize);
		return *this;
	}

	void Do(const int Init, const int Target, const int Increment, const std::function<void(int)>& Function)
	{
		//class defaults
		numThreads.OptionalSet(pSingleton::Get().NumThreads);
		bNoWait.OptionalSet(false);
		bExecuteOnMaster.OptionalSet(true);
		chunkSize.OptionalSet(1);
		schedule.OptionalSet(pSchedule::Static);

		//dynamic params
		Data = new pForChunkData(Init, Target, Increment, chunkSize.Get());
		if (schedule.Get() != pSchedule::Guided);

		//todo: split task into chunks
	}

	void CleanupThreads()
	{
		for (int i = 0; i < actualNumThreads; ++i)
		{
			threads[i]->join();
			delete threads[i];
		}
		delete[] threads;
	}
};

#define yes true
#define no false
#define parallel_do(tag, params, method) pDo tag; tag params .Do([] method);
#define parallel_for(tag, iterator, init_val, max_val, increment, params, method) pFor tag; tag params .Do(init_val, max_val, increment, [](int iterator) method);
#define num_threads(n)                    .NumThreads(n)
#define exec_master(c)                  .ExecuteOnMaster(c)
#define nowait(c)                       .NoWait(c)

#define as ,
#define create_public(type, variable)  pVariableShared<type> variable();
#define make_public(variable, type, asvariable)  pVariableShared<types> asvariable(variable);

#define create_private(type, var) pVariablePrivate<type> var
#define make_private(variable, type, asvariable) pVariablePrivate<type> var
#define last_private(variable, type, asvariable) pVariablePrivate<type> var

int main()
{

}