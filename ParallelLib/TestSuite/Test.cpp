#include "stdafx.h"
#include "Test.h"
#include <chrono>

//C-style tostring function

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

//------------------------ test result
int TestResult::GetNumTestRepeatitions()
{
	return perTryResult.size();
}

int TestResult::GetAverageResultTime()
{
	int totalTime = 0;
	for (const int& time : perTryResult) totalTime += time;
	totalTime /= GetNumTestRepeatitions();
	return totalTime;
}

bool TestResult::DidTestFail()
{
	return (testPhase != TestPhase::TaskEndedSuccessfully);
}

TestResult::TestResult(const TargetLibrary _testedLibrary)
	: testedLibrary(_testedLibrary)
	, testPhase(TestPhase::InitializedTestCase)
{
}

//------------------------ test
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

			auto start = std::chrono::steady_clock::now();
			switch (lib)
			{
			case TargetLibrary::OpenMP:
				DoOpenMP(inParams, result);
				break;
			case TargetLibrary::ParallelLib:
				DoParallelLib(inParams, result);
				break;
			case TargetLibrary::Boost:
				DoBoost(inParams, result);
				break;
			}
			auto end = std::chrono::steady_clock::now();
			long long duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
			result.perTryResult.push_back(duration);

			if (inParams.bVerboseStats)
			{
				printf("%d", duration);
			}
		}
		results.push_back(result);
	}
}

void Test::DoParallelLib(const TestParams& In, TestResult& Out)
{
	throw; //base class cannot be tested
}

void Test::DoOpenMP(const TestParams& In, TestResult& Out)
{
	throw; //base class cannot be tested
}

void Test::DoBoost(const TestParams& In, TestResult& Out)
{
	throw; //base class cannot be tested
}

TestType Test::GetTestType()
{
	return type;
}

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
