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
	
	while(true)
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
					//const std::lock_guard<std::mutex> lock_index(searchedIndexLock);
					searchedIndexLock.lock();
					currentSearchedIndex++;
					localSearchedIndex = currentSearchedIndex;
					searchedIndexLock.unlock();
				}

				if (localSearchedIndex > testConfig.searchRangeMax) break;

				if (PrimeTest::IsPrime(localSearchedIndex))
				{
					//const std::lock_guard<std::mutex> lock_primes(primesLock);
					primesLock.lock();
					primes.push_back(localSearchedIndex);
					primesLock.unlock();
				}
			}
		}
	);

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}

/*

struct data
{
	std::vector<int> primes;
	int currentSearchedIndex;
	int maxR;
	dlib::mutex primesLock;
	dlib::mutex searchedIndexLock;
	dlib::mutex threadLock;
	dlib::signaler threadSignaler;

	data(int minRange, int maxRange)
		: threadSignaler(dlib::signaler(threadLock))
		, currentSearchedIndex(minRange)
		, maxR(maxRange)
	{ }
};
void dlibPrime(void* something)
{
	data* dlib_data = (data*)something;
	while (true)
	{
		int localSearchedIndex;
		{
			//const std::lock_guard<std::mutex> lock_index(searchedIndexLock);
			dlib_data->searchedIndexLock.lock();
			dlib_data->currentSearchedIndex++;
			localSearchedIndex = dlib_data->currentSearchedIndex;
			dlib_data->searchedIndexLock.unlock();
		}

		if (localSearchedIndex > dlib_data->maxR) break;

		if (IsPrime(localSearchedIndex))
		{
			//const std::lock_guard<std::mutex> lock_primes(primesLock);
			dlib_data->primesLock.lock();
			dlib_data->primes.push_back(dlib_data->localSearchedIndex);
			dlib_data->primesLock.unlock();
		}
	}
}

#include <dlib/threads.h>
void PrimeTest::DoDlib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	data dlib_data(testConfig.searchRangeMin, testConfig.searchRangeMax);

	Out.BeginParallelWorkload();

	for (int i = 0; i < In.numThreadsToUse; ++i)
	{
		dlib::create_new_thread(dlibPrime, (void*)&dlib_data);
	}

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}
*/

#include "ParallelLib/ParallelLib.h"
void PrimeTest::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	std::vector<int> primes;
	int currentSearchedIndex = testConfig.searchRangeMin;
	std::mutex primesLock;
	std::mutex searchedIndexLock;

	Out.BeginParallelWorkload();

	parallel_do(doPrimes, num_threads(In.numThreadsToUse),
		{
			while (true)
			{
				int localSearchedIndex;
				{
					//const std::lock_guard<std::mutex> lock_index(searchedIndexLock);
					searchedIndexLock.lock();
					currentSearchedIndex++;
					localSearchedIndex = currentSearchedIndex;
					searchedIndexLock.unlock();
				}

				if (localSearchedIndex > testConfig.searchRangeMax) break;

				if (IsPrime(localSearchedIndex))
				{
					//const std::lock_guard<std::mutex> lock_primes(primesLock);
					primesLock.lock();
					primes.push_back(localSearchedIndex);
					primesLock.unlock();
				}
			}
		});

	Out.BeginResourceCleanup();
	//printf("pll-Result: %d primes. Last prime %d\n", primes.size(), primes.back());
	Out.EndTask(true);
}
#undef parallel_do