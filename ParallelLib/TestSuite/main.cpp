#include "stdafx.h"
#include "MandelbrotTest.h"

int InitLibraries()
{
	//Intel TBB

}

int main()
{
	std::vector<TargetLibrary> testedLibsStatic  = { TargetLibrary::NoLibrary, TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB };
	std::vector<TargetLibrary> testedLibsDynamic = {                           TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB };
	std::vector<TargetLibrary> testedLibsGuided  = {                           TargetLibrary::OpenMP,                                                     };

	const int numTestRepeatitions = 1;
	const char* testName = "Mandelbrot";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	for (int iSched = 0; iSched < 3; ++iSched)
	{
		for (int iNumThreads = 1; iNumThreads < 9; ++iNumThreads)
		{
			const int actualChunkSize = 16;

			MandelbrotTest test = MandelbrotTest(testName);
			TestParams config(
				false,               //const bool _bVerboseStats,
				1,                   //const int _numTestRepeatitions,
				true,                //const bool _bVerboseTest,
				iNumThreads,         //const int _numThreadsToUse,
				(ForSchedule)iSched, //const ForSchedule _forSchedule,
				actualChunkSize,     //const int forChunkSize,
				nullptr              //void* _userData
			);

			std::vector<TargetLibrary> testedLibs;
			switch (iSched)
			{
			case 0: testedLibs = testedLibsStatic ; break;
			case 1: testedLibs = testedLibsDynamic; break;
			case 2: testedLibs = testedLibsGuided;  break;
			}

			std::vector<TestResult> perLibraryResults;
			test.PerformTests(testedLibs, config, perLibraryResults);

			for (const TestResult& result : perLibraryResults)
			{
				printf("%s\t%s\t%d\t%d\t%s\t%llu\n"
					, LibraryToString(result.testedLibrary)
					, ForScheduleToString((ForSchedule)iSched)
					, iNumThreads
					, actualChunkSize
					, result.DidTestSucceed() ? "succeeded" : "failed"
					, result.GetAverageResultTime());
			}
		}
	}

	getchar();
	return 0;

	for (int iSched = 0; iSched < 1; ++iSched)
	{
		for (int iChunk = 0; iChunk < 6; ++iChunk)
		{
			int actualChunkSize = (1 << iChunk);

			MandelbrotTest test = MandelbrotTest("Mandelbrot");
			TestParams config(
				true,                //const bool _bVerboseStats,
				4,                   //const int _numTestRepeatitions,
				true,                //const bool _bVerboseTest,
				2,                   //const int _numThreadsToUse,
				(ForSchedule)iSched, //const ForSchedule _forSchedule,
				actualChunkSize,     //const int forChunkSize,
				nullptr              //void* _userData
			);

			//std::vector<TargetLibrary> testedLibs;
			//switch (iSched)
			//{
			//case 0: testedLibs = iChunk == 0 ? testedLibsStatic : testedLibsDynamic;  break;
			//case 1: testedLibs = testedLibsDynamic;                                   break;
			//case 2: testedLibs = testedLibsGuided;                                    break;
			//}

			std::vector<TestResult> perLibraryResults;
			test.PerformTests(/*testedLibs*/ testedLibsStatic, config, perLibraryResults);

			for (const TestResult& result : perLibraryResults)
			{
				printf("Tested %s with %s (num retries: %d, sched: %s, chunksize: %d)\n", test.GetTestName().c_str(), LibraryToString(result.testedLibrary), (int32_t)result.GetNumTestRepeatitions(), ForScheduleToString((ForSchedule)iSched), actualChunkSize);
				printf("\tThe test %s with average time of %llu nanoseconds.\n\n", result.DidTestSucceed() ? "succeeded" : "failed", result.GetAverageResultTime());
			}
		}
	}

	getchar();
    return 0;
}
