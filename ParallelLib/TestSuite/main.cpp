#include "stdafx.h"
#include "MandelbrotTest.h"

int InitLibraries()
{
	//Intel TBB

}

int main()
{
	std::vector<TargetLibrary> sequentialTest    = { TargetLibrary::NoLibrary };
	std::vector<TargetLibrary> testedLibsStatic  = { TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB, TargetLibrary::dlib };
	std::vector<TargetLibrary> testedLibsDynamic = { TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB,                     };
	std::vector<TargetLibrary> testedLibsGuided  = { TargetLibrary::OpenMP,                                                                          };

	const int numTestRepeatitions = 1;
	const char* testName = "Mandelbrot";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	for (int iSched = -1; iSched < 3; ++iSched)
	{
		std::vector<TargetLibrary> testedLibs;
		switch (iSched)
		{
		case -1: testedLibs = sequentialTest;    break;
		case 0 : testedLibs = testedLibsStatic;  break;
		case 1 : testedLibs = testedLibsDynamic; break;
		case 2 : testedLibs = testedLibsGuided;  break;
		}

		for (int iNumThreads = 1; iNumThreads < 9; ++iNumThreads)
		{
			if (iSched == -1 && iNumThreads > 1) continue;
			if (iSched != -1 && iNumThreads < 2) continue;

			for (int iChunk = 0; iChunk < 6; ++iChunk)
			{
				if (iSched == -1 && iChunk > 0) continue;

				const int actualChunkSize = (1 << iChunk);

				MandelbrotTest test = MandelbrotTest(testName);
				TestParams config(
					false,               //const bool _bVerboseStats,
					10,                  //const int _numTestRepeatitions,
					true,                //const bool _bVerboseTest,
					iNumThreads,         //const int _numThreadsToUse,
					(ForSchedule)iSched, //const ForSchedule _forSchedule,
					actualChunkSize,     //const int forChunkSize,
					nullptr              //void* _userData
				);

				std::vector<TestResult> perLibraryResults;
				test.PerformTests(testedLibs, config, perLibraryResults);

				for (const TestResult& result : perLibraryResults)
				{
					printf("%s\t%s\t%d\t%d\t%s\t%llu\n"
						, LibraryToString(result.testedLibrary)
						, ForScheduleToString((ForSchedule)(iSched >= 0 ? iSched : 255))
						, iNumThreads
						, actualChunkSize
						, result.DidTestSucceed() ? "succeeded" : "failed"
						, result.GetAverageResultTime());
				}
			}
		}
	}

	getchar();
	return 0;
}
