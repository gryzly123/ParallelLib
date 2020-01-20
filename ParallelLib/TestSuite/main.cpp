#include "stdafx.h"
#include "PrioritySetter.h"

#include "MandelbrotTest.h"
#include "MatrixTest.h"
#include "PrimeTest.h"
#include "StringTest.h"

struct ForTestVariant
{
	const TargetLibrary targetLibrary;
	const ForSchedule targetSchedule;
	const std::vector<int>& targetChunkSize;
	const std::vector<int>& numThreads;

	ForTestVariant(TargetLibrary targetLibrary, ForSchedule targetSchedule, std::vector<int>& targetChunkSize, std::vector<int>& numThreads)
		: targetLibrary(targetLibrary), targetSchedule(targetSchedule), targetChunkSize(targetChunkSize), numThreads(numThreads) { }
};

#define DOUBLE_PRINT(printable) { std::cout << printable; printBuffer << printable; }

int mandelbrot(const int numTestRepeatitions, const int maxNumTestedThreads, const int maxTestedChunkPower, std::ostream& printBuffer)
{
	//Test variants
	// * Sequential
	// * OpenMP Static: chunks 1-32
	// * OpenMP Dynamic: chunks 1-32
	// * OpenMP Guided
	// * ParallelLib Static
	// * ParallelLib Dynamic
	// * TBB Static
	// * TBB Default ("dynamic")
	// * dlib Default ("dynamic"): chunks 1-32

	std::vector<int> sequentialThreads = { 1 };
	std::vector<int> parallelThreads;
	for (int i = 2; i <= maxNumTestedThreads; ++i) parallelThreads.push_back(i);

	std::vector<int> unsupportedChunks = { 0 };
	std::vector<int> supportedChunks;
	for (int i = 0; i <= maxTestedChunkPower; ++i) supportedChunks.push_back(1 << i);

	std::vector<ForTestVariant> variants =
	{
		ForTestVariant(TargetLibrary::NoLibrary,   ForSchedule::None,    unsupportedChunks, sequentialThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Static,    supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Dynamic,   supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Guided,  unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::ParallelLib, ForSchedule::Static,  unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::ParallelLib, ForSchedule::Dynamic,   supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::IntelTBB,    ForSchedule::Static,  unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::IntelTBB,    ForSchedule::Dynamic, unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::dlib,        ForSchedule::Dynamic,   supportedChunks, parallelThreads)
	};

	const char* testName = "Mandelbrot";
	DOUBLE_PRINT("Tested" << testName << " (num retries: " << numTestRepeatitions << ")\n");
	DOUBLE_PRINT("LIB\tSCHED\tNUM_THR\tCHUNK_SIZE\tSUCC\tAVG_TIME\n");

	std::vector<TestResult> fullResults;

	for (const ForTestVariant& variant : variants)
	{
		for (const int& numThreads : variant.numThreads)
		{
			for (const int& targetChunkSize : variant.targetChunkSize)
			{
				MandelbrotTest test = MandelbrotTest(testName);
				TestParams config(
					false,                   //const bool _bVerboseStats,
					numTestRepeatitions,     //const int _numTestRepeatitions,
					true,                    //const bool _bVerboseTest,
					numThreads,              //const int _numThreadsToUse,
					variant.targetSchedule,  //const ForSchedule _forSchedule,
					targetChunkSize,         //const int forChunkSize,
					nullptr                  //void* _userData
				);

				std::vector<TestResult> perLibraryResults;
				test.PerformTests({ variant.targetLibrary }, config, perLibraryResults);

				const TestResult& result = perLibraryResults[0];
				DOUBLE_PRINT(
					LibraryToString(result.testedLibrary)
					<< "\t" << ForScheduleToString(variant.targetSchedule)
					<< "\t" << numThreads
					<< "\t" << targetChunkSize
					<< "\t" << (result.DidTestSucceed() ? 1 : 0)
					<< "\t" << result.GetAverageResultTime()
					<< "\n");
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
		case 1: 
		{
			std::ofstream mandel_file("mandelbrot_result.txt");
			mandelbrot(10, 8, 5, mandel_file); 
		} break;

		case 2: matrix(); break;
		case 3: primes(); break;
		case 4: string(); break;
		default: return 0;
		}
	}
}
