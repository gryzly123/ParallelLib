// TestSuite.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include "Test.h"
#include "MandelbrotTest.h"


int main()
{
	std::vector<TargetLibrary> testedLibs = { TargetLibrary::NoLibrary, TargetLibrary::OpenMP, TargetLibrary::ParallelLib };

	MandelbrotTest test = MandelbrotTest("mandelbrot1");
	TestParams config(
		true,                //const bool _bVerboseStats,
		10,                   //const int _numTestRepeatitions,
		true,                //const bool _bVerboseTest,
		4,                   //const int _numThreadsToUse,
		ForSchedule::Static, //const ForSchedule _forSchedule,
		nullptr              //void* _userData
	);

	std::vector<TestResult> perLibraryResults;
	test.PerformTests(testedLibs, config, perLibraryResults);

	for (const TestResult& result : perLibraryResults)
	{
		printf("Tested %s with %s (num retries: %d)\n", test.GetTestName(), LibraryToString(result.testedLibrary), result.GetNumTestRepeatitions());
		printf("\tThe test %s with average time of %llu nanoseconds.\n\n", result.DidTestSucceed() ? "succeeded" : "failed", result.GetAverageResultTime());
	}

	getchar();
    return 0;
}
