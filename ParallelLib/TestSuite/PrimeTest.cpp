#include "stdafx.h"
#include "PrimeTest.h"

PrimeTestConfig::PrimeTestConfig() { }

PrimeTestConfig::PrimeTestConfig(int searchRangeMin, int searchRangeMax)
	: searchRangeMin(searchRangeMin)
	, searchRangeMax(searchRangeMax)
{
	if (searchRangeMin < 0 || searchRangeMax < searchRangeMin) throw;
}

PrimeTest::PrimeTest(const std::string& name, PrimeTestConfig& testConfig)
	: Test(TestType::PrimeNumbers, name)
	, testConfig(testConfig)
{ }

bool PrimeTest::IsPrime(int index)
{
	int maxPotentialDividor = (int)std::sqrt(index);
	for (int i = 2; i <= maxPotentialDividor; ++i) if (index % i == 0) return false;
	return true;
}

void PrimeTest::DoSequentially(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	
	std::vector<int> primes;
	int currentSearchedIndex = testConfig.searchRangeMin;
	
	Out.BeginParallelWorkload();
	
	while (true)
	{
		int localSearchedIndex;
		localSearchedIndex = currentSearchedIndex++;
		if (localSearchedIndex > testConfig.searchRangeMax) break;
		if (IsPrime(localSearchedIndex)) primes.push_back(localSearchedIndex);
	}
	
	Out.BeginResourceCleanup();
	//printf("seq--Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}

void PrimeTest::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();
	
	std::vector<int> primes;
	int currentSearchedIndex = testConfig.searchRangeMin;
	
	Out.BeginParallelWorkload();
	
	#pragma omp parallel num_threads(In.numThreadsToUse)
	{
		while (true)
		{
			int localSearchedIndex;
			#pragma omp critical
			{
				currentSearchedIndex++;
				localSearchedIndex = currentSearchedIndex;
			}
	
			if (localSearchedIndex > testConfig.searchRangeMax) break;
	
			if (IsPrime(localSearchedIndex))
			{
				#pragma omp critical
				{
					primes.push_back(localSearchedIndex);
				}
			}
		}
	}
	
	Out.BeginResourceCleanup();
	//printf("omp--Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}

#include "tbb/iterators.h"
#include "tbb/mutex.h"
#include "tbb/parallel_do.h"
//https://software.intel.com/en-us/node/506067 15.01.2020

void PrimeTest::DoTBB(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	std::vector<int> primes;
	int currentSearchedIndex = testConfig.searchRangeMin;
	tbb::mutex primesLock;
	tbb::mutex searchedIndexLock;

	Out.BeginParallelWorkload();

	tbb::parallel_do(
		tbb::counting_iterator<int>(0),
		tbb::counting_iterator<int>(In.numThreadsToUse),
		[&](int thread_id, tbb::parallel_do_feeder<int>& feeder)
		{
			while (true)
			{
				int localSearchedIndex;
				{
					tbb::mutex::scoped_lock lock(searchedIndexLock);
					currentSearchedIndex++;
					localSearchedIndex = currentSearchedIndex;
				}

				if (localSearchedIndex > testConfig.searchRangeMax) break;

				if (PrimeTest::IsPrime(localSearchedIndex))
				{
					tbb::mutex::scoped_lock lock(primesLock);
					primes.push_back(localSearchedIndex);
				}
			}
		}
	);

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}


#include <dlib/threads.h>   //for dlib::mutex, signaler, thread
#include <dlib/misc_api.h>  //for dlib::sleep
struct data
{
	std::vector<int> primes;
	int currentSearchedIndex;
	int maxR;
	dlib::mutex primesLock;
	dlib::mutex searchedIndexLock;
	dlib::mutex threadLock;
	int numThreads;
	dlib::signaler threadSignaler;

	data(int minRange, int maxRange, int numThreads)
		: threadSignaler(threadLock)
		, currentSearchedIndex(minRange)
		, maxR(maxRange)
		, numThreads(numThreads)
	{ }

	void pop_thread()
	{
		threadLock.lock();
		--numThreads;
		threadLock.unlock();
	}

	bool is_done()
	{
		return (numThreads == 0);
	}
};
void dlibPrime(void* something)
{
	data* dlib_data = (data*)something;
	while (true)
	{
		int localSearchedIndex;
		{
			dlib_data->searchedIndexLock.lock();
			dlib_data->currentSearchedIndex++;
			localSearchedIndex = dlib_data->currentSearchedIndex;
			dlib_data->searchedIndexLock.unlock();
		}

		if (localSearchedIndex > dlib_data->maxR)
		{
			dlib_data->pop_thread();
			break;
		}

		if (PrimeTest::IsPrime(localSearchedIndex))
		{
			dlib_data->primesLock.lock();
			dlib_data->primes.push_back(localSearchedIndex);
			dlib_data->primesLock.unlock();
		}
	}
}
void PrimeTest::DoDlib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	data dlib_data(testConfig.searchRangeMin, testConfig.searchRangeMax, In.numThreadsToUse);

	Out.BeginParallelWorkload();

	for (int i = 0; i < In.numThreadsToUse; ++i)
	{
		dlib::create_new_thread(dlibPrime, (void*)&dlib_data);
	}
	while (!dlib_data.is_done()) dlib::sleep(1);

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}


#include "ParallelLib/ParallelLib.h"
void PrimeTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	std::vector<int> primes;
	int currentSearchedIndex = testConfig.searchRangeMin;
	std::mutex primesLock;
	std::mutex searchedIndexLock;

	Out.BeginParallelWorkload();

	#define NUM 500
	pDo tag; tag.NumThreads(In.numThreadsToUse).Do([&](pExecParams ___pExecParams) 
	{
		int checkIndexesMin = 0, checkIndexesMax = 0, foundIndexesNum = 0;
		int foundIndexes[NUM];
		while (true)
		{
			{
				const std::lock_guard<std::mutex> lock_index(searchedIndexLock);
				if (currentSearchedIndex >= testConfig.searchRangeMax) break;
				checkIndexesMin = ++currentSearchedIndex;
				checkIndexesMax = std::min<int>(checkIndexesMin + NUM, testConfig.searchRangeMax);
				currentSearchedIndex = checkIndexesMax + 1;
			}

			foundIndexesNum = 0;
			for (int i = checkIndexesMin; i <= checkIndexesMax; ++i)
				if (IsPrime(i))
					foundIndexes[foundIndexesNum++] = i;

			{
				const std::lock_guard<std::mutex> lock_primes(primesLock);
				for (int i = 0; i < foundIndexesNum; ++i)
					primes.push_back(i);
			}
		}
	});

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}
#undef parallel_do
