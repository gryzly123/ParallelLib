#include "stdafx.h"
#define IN_TEST_CPP 1
#include "Test.h"
#include "tbb/task_scheduler_init.h"

// ----------------------- ENUMS -----------------------

const char* LibraryToString(TargetLibrary Library)
{
	switch (Library)
	{
	case TargetLibrary::NoLibrary:   return "Sequential";
	case TargetLibrary::OpenMP:      return "OpenMP";
	case TargetLibrary::ParallelLib: return "ParallelLib";
	case TargetLibrary::IntelTBB:    return "IntelTBB";
	case TargetLibrary::dlib:        return "dlib";
	//case TargetLibrary::Boost:       return "Boost";
	//case TargetLibrary::MicrosoftPPL:       return "Microsoft PPL";
	}
	throw; //unsupported library
	return "";
}

const char* ForScheduleToString(ForSchedule Schedule)
{
	switch (Schedule)
	{
	case ForSchedule::Static:  return "static";
	case ForSchedule::Dynamic: return "dynamic";
	case ForSchedule::Guided:  return "guided";
	case ForSchedule::None:    return "none";
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
	const int _forChunkSize,
	void* _userData)
	: bVerboseStats(_bVerboseStats)
	, numTestRepeatitions(_numTestRepeatitions)
	, bVerboseTest(_bVerboseTest)
	, numThreadsToUse(_numThreadsToUse)
	, forSchedule(_forSchedule)
	, forChunkSize(_forChunkSize)
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

void RetryResult::BeginParallelWorkload()
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

size_t TestResult::GetNumTestRepeatitions() const
{
	return perTryResults.size();
}

TimeSpan TestResult::GetAverageResultTime() const
{
	TimeSpan totalTime = 0;
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

bool TestResult::DidTestSucceed() const
{
	for (const RetryResult& result : perTryResults)
		if (!result.GetTaskSucceeded())
			return false;

	return true;
}

// --------------------- TEST CLASS ---------------------

Test::Test() : type(TestType::None) { throw; } //this should never be used
Test::Test(TestType inType, const std::string& inName) : type(inType), name(inName), testNum(0) { }

Test::~Test() { }

void Test::PerformTests(std::vector<TargetLibrary> targetLibs, const TestParams& inParams, std::vector<TestResult>& results)
{
	results.empty();
	for (const TargetLibrary lib : targetLibs)
	{
		runningLibrary = lib;

		if (inParams.bVerboseStats)
		{
			printf("Testing library %s:\n", LibraryToString(lib));
		}

		//Create the TestResult object for the current library.
		TestResult result(lib);

		for (int i = 0; i < inParams.numTestRepeatitions; ++i)
		{
			testNum = i;

			if (inParams.bVerboseStats)
			{
				printf("\tTest %d:\t", i);
			}

			//Create the RetryResult object for current test instance.
			RetryResult retryResult;
			retryResult.userData = result.userData;

			switch (lib)
			{
			case TargetLibrary::NoLibrary:
				DoSequentially(inParams, retryResult);
				break;
			case TargetLibrary::OpenMP:
				DoOpenMP(inParams, retryResult);
				break;
			case TargetLibrary::ParallelLib:
				DoParallelLib(inParams, retryResult);
				break;
			case TargetLibrary::IntelTBB:

				//in IntelTBB's case we manually reset the task scheduling before starting the task.
				//this ensures that the required threads are preallocated and there are as many
				//threads as defined in TestParams object.
				{
					tbb::task_scheduler_init tbbScheduler(inParams.numThreadsToUse);
					DoTBB(inParams, retryResult);
				}
				break;
			case TargetLibrary::dlib:
				DoDlib(inParams, retryResult);
				break;

			//case TargetLibrary::Boost:
			//	DoBoost(inParams, retryResult);
			//	break;
			default:
				throw; //unsupported library
			}

			if (inParams.bVerboseStats)
			{
				if (retryResult.GetTaskSucceeded())
				{
					printf("succ\t%llu\t%llu\t%llu\n",
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

void Test::DoSequentially(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
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

void Test::DoTBB(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
}

void Test::DoDlib(const TestParams& In, RetryResult& Out)
{
	throw; //base class cannot be tested
}
