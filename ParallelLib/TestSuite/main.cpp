#include "stdafx.h"
#include "PrioritySetter.h"

#include "MandelbrotTest.h"
#include "MatrixTest.h"
#include "PrimeTest.h"
#include "StringTest.h"

int mandelbrot()
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

	return 0;
}

int matrix()
{
	std::vector<TargetLibrary> sequentialTest = { TargetLibrary::NoLibrary, TargetLibrary::ParallelLib, TargetLibrary::OpenMP, TargetLibrary::IntelTBB, TargetLibrary::dlib };

	const int numTestRepeatitions = 1;
	const char* testName = "MatrixMul";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	for (int i = 0; i < 2; ++i)
	{
		MatrixTestConfig matrixConfig = MatrixTestConfig((i == 1), false);
		MatrixTest test = MatrixTest(testName, matrixConfig);
		TestParams config(
			false,               //const bool _bVerboseStats,
			10,                  //const int _numTestRepeatitions,
			true,                //const bool _bVerboseTest,
			4,                   //const int _numThreadsToUse,
			ForSchedule::Static, //const ForSchedule _forSchedule,
			10,                  //const int forChunkSize,
			nullptr              //void* _userData
		);

		std::vector<TestResult> perLibraryResults;
		test.PerformTests(sequentialTest, config, perLibraryResults);

		for (const TestResult& result : perLibraryResults)
		{
			printf("%s\t%s\t%d\t%d\t%s\t%llu\n"
				, LibraryToString(result.testedLibrary)
				, ForScheduleToString(ForSchedule::None)
				, 1
				, 1
				, result.DidTestSucceed() ? "succeeded" : "failed"
				, result.GetAverageResultTime());
		}
	}

	return 0;
}

int primes()
{
	PrioritySetter::SetPriority(Priority::Realtime);

	std::vector<TargetLibrary> tests1 = { TargetLibrary::NoLibrary };
	std::vector<TargetLibrary> testsm = { TargetLibrary::ParallelLib, TargetLibrary::OpenMP, TargetLibrary::IntelTBB, TargetLibrary::dlib };

	const int numTestRepeatitions = 5;
	const char* testName = "PrimeTest";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	for (int i = 1; i < 9; ++i)
	{
		PrimeTestConfig primeConfig = PrimeTestConfig();
		PrimeTest test = PrimeTest(testName, primeConfig);
		TestParams config(
			false,               //const bool _bVerboseStats,
			numTestRepeatitions, //const int _numTestRepeatitions,
			true,                //const bool _bVerboseTest,
			i,                   //const int _numThreadsToUse,
			ForSchedule::Static, //const ForSchedule _forSchedule,
			10,                  //const int forChunkSize,
			nullptr              //void* _userData
		);

		std::vector<TestResult> perLibraryResults;
		test.PerformTests(i == 1 ? tests1 : testsm, config, perLibraryResults);

		for (const TestResult& result : perLibraryResults)
		{
			printf("%s\t%s\t%d\t%d\t%s\t%llu\n"
				, LibraryToString(result.testedLibrary)
				, ForScheduleToString(ForSchedule::None)
				, i
				, 0
				, result.DidTestSucceed() ? "succeeded" : "failed"
				, result.GetAverageResultTime());
		}
	}

	return 0;
}

int string()
{
	std::vector<TargetLibrary> tests = { TargetLibrary::NoLibrary, TargetLibrary::ParallelLib, TargetLibrary::OpenMP, TargetLibrary::IntelTBB };

	const int numTestRepeatitions = 25;
	const char* testName = "PrimeTest";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	StringTestConfig primeConfig = StringTestConfig(5000, 10000);
	StringTest test = StringTest(testName, primeConfig);
	TestParams config(
		false,               //const bool _bVerboseStats,
		numTestRepeatitions, //const int _numTestRepeatitions,
		true,                //const bool _bVerboseTest,
		4,                   //const int _numThreadsToUse,
		ForSchedule::Static, //const ForSchedule _forSchedule,
		10,                  //const int forChunkSize,
		nullptr              //void* _userData
	);

	std::vector<TestResult> perLibraryResults;
	test.PerformTests(tests, config, perLibraryResults);

	for (const TestResult& result : perLibraryResults)
	{
		printf("%s\t%s\t%d\t%d\t%s\t%llu\n"
			, LibraryToString(result.testedLibrary)
			, ForScheduleToString(ForSchedule::None)
			, 4
			, 0
			, result.DidTestSucceed() ? "succeeded" : "failed"
			, result.GetAverageResultTime());
	}
	

	return 0;
}

int main()
{
	PrioritySetter::SetPriority(Priority::Realtime);

	while (true)
	{
		unsigned int targetTest = 0;
		std::cout << "Select test (1=mandelbrot, 2=matrix, 3=prime, 4=string): ";
		std::cin >> targetTest;
		switch (targetTest)
		{
		case 1: mandelbrot(); break;
		case 2: matrix(); break;
		case 3: primes(); break;
		case 4: string(); break;
		default: return 0;
		}
	}
}
