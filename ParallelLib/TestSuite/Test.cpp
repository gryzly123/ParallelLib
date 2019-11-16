#include "stdafx.h"
#include "Test.h"

// ----------------------- ENUMS -----------------------

const char* LibraryToString(TargetLibrary Library)
{
	switch (Library)
	{
	case TargetLibrary::OpenMP:      return "OpenMP";
	case TargetLibrary::ParallelLib: return "ParallelLib";
	case TargetLibrary::Boost:       return "Boost";
	}
	throw; //unsupported library
	return "";
}

// ----------------------- STRUCTS -----------------------

//------------------------ TEST PARAMS
TestParams::TestParams(
	const bool _bVerboseStats,
	const int _numTestRepeatitions,
	const bool _bVerboseTest,
	const int _numThreadsToUse,
	const ForSchedule _forSchedule,
	void* _userData)
	: bVerboseStats(_bVerboseStats)
	, numTestRepeatitions(_numTestRepeatitions)
	, bVerboseTest(_bVerboseTest)
	, numThreadsToUse(_numThreadsToUse)
	, forSchedule(_forSchedule)
	, userData(_userData)
{ }

//------------------------ RETRY RESULT
RetryResult::RetryResult() :testState(TestPhase::InitializedTestCase) { }

void RetryResult::BeginResourceInit()
{
	if (testState != TestPhase::InitializedTestCase) throw;
	testState = TestPhase::BegunResourceInitialization;
	start = std::chrono::steady_clock::now();
}

void RetryResult::BeginParallWorkload()
{
	if (testState != TestPhase::BegunResourceInitialization) throw;

	TimeStamp end = std::chrono::steady_clock::now();
	phaseTime[0] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	start = std::chrono::steady_clock::now();
	testState = TestPhase::BegunParallelWorkload;
}

void RetryResult::BeginResourceCleanup()
{
	if (testState != TestPhase::BegunParallelWorkload) throw;

	TimeStamp end = std::chrono::steady_clock::now();
	phaseTime[1] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	start = std::chrono::steady_clock::now();
	testState = TestPhase::BegunResourceCleanup;
}

void RetryResult::EndTask(bool bSucceeded)
{
	if (bSucceeded)
	{
		if (testState != TestPhase::BegunResourceCleanup) throw;
		TimeStamp end = std::chrono::steady_clock::now();
		phaseTime[2] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	}
	testState = bSucceeded ? TestPhase::TaskEndedSuccessfully : TestPhase::TaskFailed;
}

//------------------------ TEST RESULT
TestResult::TestResult(const TargetLibrary _testedLibrary)
	: testedLibrary(_testedLibrary)
{ }

int TestResult::GetNumTestRepeatitions() const
{
	return perTryResults.size();
}

int TestResult::GetAverageResultTime() const
{
	int totalTime = 0;
	int numSucceededRetries = 0;
	for (const RetryResult& result : perTryResults)
	{
		if (!result.GetTaskSucceeded()) continue;
		++numSucceededRetries;
		totalTime += result.GetParallelWorkloadDuration();
	}

	if (numSucceededRetries == 0) return 0;
	totalTime /= numSucceededRetries;
	return totalTime;
}

bool TestResult::DidTestFail() const
{
	for (const RetryResult& result : perTryResults)
		if (!result.GetTaskSucceeded())
			return false;

	return true;
}

// --------------------- TEST CLASS ---------------------

Test::Test() : type(TestType::None) { }

Test::~Test() { }

void Test::PerformTests(std::vector<TargetLibrary> targetLibs, const TestParams& inParams, std::vector<TestResult>& results)
{
	results.empty();
	for (const TargetLibrary& lib : targetLibs)
	{
		if (inParams.bVerboseStats)
		{
			printf("Testing library %s:\n", LibraryToString(lib));
		}

		//Create the TestResult object for the current library.
		TestResult result(lib);

		for (int i = 0; i < inParams.numTestRepeatitions; ++i)
		{
			if (inParams.bVerboseStats)
			{
				printf("\tTest %d:\t", i);
			}

			//Create the RetryResult object for current test instance.
			RetryResult retryResult;
			retryResult.userData = result.userData;

			switch (lib)
			{
			case TargetLibrary::OpenMP:
				DoOpenMP(inParams, retryResult);
				break;
			case TargetLibrary::ParallelLib:
				DoParallelLib(inParams, retryResult);
				break;
			case TargetLibrary::Boost:
				DoBoost(inParams, retryResult);
				break;
			}

			if (inParams.bVerboseStats)
			{
				if (retryResult.GetTaskSucceeded())
				{
					printf("succ\t%d\t%d\t%d\n",
						retryResult.GetResourceInitDuration(),
						retryResult.GetParallelWorkloadDuration(),
						retryResult.GetResourceCleanupDuration()
					);
				}
				else
				{
				printf("fail\n");
				}
			}

			result.perTryResults.push_back(retryResult);
		}
		results.push_back(result);
	}
}

void Test::DoParallelLib(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
}

void Test::DoOpenMP(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
}

void Test::DoBoost(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
}
