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
				localSearchedIndex = currentSearchedIndex++;
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
