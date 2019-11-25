#include "stdafx.h"
#include "MandelbrotTest.h"

int main()
{
	std::vector<TargetLibrary> testedLibsStatic  = { TargetLibrary::NoLibrary, TargetLibrary::OpenMP, TargetLibrary::ParallelLib };
	std::vector<TargetLibrary> testedLibsDynamic = {                           TargetLibrary::OpenMP, TargetLibrary::ParallelLib };
	std::vector<TargetLibrary> testedLibsGuided  = {                           TargetLibrary::OpenMP,                            };

	for (int iSched = 0; iSched < 3; ++iSched)
		for (int iChunk = 0; iChunk < 1024; ++iChunk)
	{
		MandelbrotTest test = MandelbrotTest("Mandelbrot");
		TestParams config(
			true,                //const bool _bVerboseStats,
			10,                  //const int _numTestRepeatitions,
			true,                //const bool _bVerboseTest,
			8,                   //const int _numThreadsToUse,
			(ForSchedule)iSched, //const ForSchedule _forSchedule,
			(1 << iChunk),       //const int forChunkSize,
			nullptr              //void* _userData
		);

		std::vector<TargetLibrary> testedLibs;
		switch (iSched)
		{
		case 0: testedLibs = iChunk == 0 ? testedLibsStatic : testedLibsDynamic;  break;
		case 1: testedLibs =               testedLibsDynamic;                     break;
		case 2: testedLibs =               testedLibsGuided;                      break;
		}

		std::vector<TestResult> perLibraryResults;
		test.PerformTests(testedLibs, config, perLibraryResults);

		for (const TestResult& result : perLibraryResults)
		{
			printf("Tested %s with %s (num retries: %d)\n", test.GetTestName(), LibraryToString(result.testedLibrary), result.GetNumTestRepeatitions());
			printf("\tThe test %s with average time of %llu nanoseconds.\n\n", result.DidTestSucceed() ? "succeeded" : "failed", result.GetAverageResultTime());
		}
	}

	getchar();
    return 0;
}
