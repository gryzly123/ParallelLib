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
	const std::vector<int> targetChunkSize;
	const std::vector<int> numThreads;

	ForTestVariant(TargetLibrary targetLibrary, ForSchedule targetSchedule, std::vector<int>& targetChunkSize, std::vector<int>& numThreads)
		: targetLibrary(targetLibrary), targetSchedule(targetSchedule), targetChunkSize(targetChunkSize), numThreads(numThreads) { }
};

std::vector<ForTestVariant> BuildForVariants(const int maxNumTestedThreads, const int maxTestedChunkPower)
{
	//Possible test variants
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
	std::vector<int> parallelThreadsTbb;
	for (int i = 2; i <= maxNumTestedThreads; ++i)
	{
		parallelThreads.push_back(i);
		if(i < 4) parallelThreadsTbb.push_back(i);
	}

	std::vector<int> unsupportedChunks = { 0 };
	std::vector<int> supportedChunks;
	for (int i = 0; i <= maxTestedChunkPower; ++i) supportedChunks.push_back(1 << i);

	return {
		ForTestVariant(TargetLibrary::NoLibrary,   ForSchedule::None,    unsupportedChunks, sequentialThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Static,    supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Dynamic,   supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::Guided,  unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::ParallelLib, ForSchedule::Static,  unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::ParallelLib, ForSchedule::Dynamic,   supportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::IntelTBB,    ForSchedule::Static,  unsupportedChunks, parallelThreadsTbb),
		ForTestVariant(TargetLibrary::IntelTBB,    ForSchedule::Dynamic, unsupportedChunks, parallelThreadsTbb),
		ForTestVariant(TargetLibrary::dlib,        ForSchedule::Dynamic,   supportedChunks, parallelThreads)
	};
}

std::vector<ForTestVariant> BuildDoVariants(const int maxNumTestedThreads)
{
	//Possible test variants are simply library types

	std::vector<int> sequentialThreads = { 1 };
	std::vector<int> parallelThreads;
	for (int i = 2; i <= maxNumTestedThreads; ++i) parallelThreads.push_back(i);

	std::vector<int> unsupportedChunks = { 0 };

	return {
		ForTestVariant(TargetLibrary::NoLibrary,   ForSchedule::None, unsupportedChunks, sequentialThreads),
		ForTestVariant(TargetLibrary::OpenMP,      ForSchedule::None, unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::ParallelLib, ForSchedule::None, unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::IntelTBB,    ForSchedule::None, unsupportedChunks, parallelThreads),
		ForTestVariant(TargetLibrary::dlib,        ForSchedule::None, unsupportedChunks, parallelThreads)
	};
}


#define DOUBLE_PRINT(printable) { std::cout << printable; printBuffer << printable; }

void mandelbrot(const std::vector<ForTestVariant>& variants, const int numTestRepeatitions, const bool bExportImages, std::ostream& printBuffer)
{
	const char* testName = "Mandelbrot";
	DOUBLE_PRINT("Tested" << testName << " (num retries: " << numTestRepeatitions << ")\n");
	DOUBLE_PRINT("LIB      \tSCHED\tNUM_THR\tCHUNK_S\tSUCC\tAVG_TIM\tSTD_DEV\n");

	for (const ForTestVariant& variant : variants)
	{
		for (const int& numThreads : variant.numThreads)
		{
			for (const int& targetChunkSize : variant.targetChunkSize)
			{
				MandelbrotTestConfig testConfig(bExportImages);
				MandelbrotTest test = MandelbrotTest(testName, testConfig);
				TestParams config(
					false,                   //const bool _bVerboseStats,
					numTestRepeatitions,     //const int _numTestRepeatitions,
					false,                   //const bool _bVerboseTest,
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
					<< "\t" << result.GetStandardDeviation()
					<< "\n");
			}
		}
	}
}

void matrix(const std::vector<ForTestVariant>& variants, const int numTestRepeatitions, const bool bTransposeMatrix, const bool bDoubleForNesting, std::ostream& printBuffer)
{
	const char* testName = "MatrixMul";
	DOUBLE_PRINT("Tested" << testName << " (num retries: " << numTestRepeatitions << ")\n");
	DOUBLE_PRINT("LIB      \tSCHED\tNUM_THR\tCHUNK_S\tSUCC\tAVG_TIM\tSTD_DEV\n");

	for (const ForTestVariant& variant : variants)
	{
		for (const int& numThreads : variant.numThreads)
		{
			for (const int& targetChunkSize : variant.targetChunkSize)
			{
				MatrixTestConfig matrixConfig = MatrixTestConfig(bTransposeMatrix, bDoubleForNesting);
				MatrixTest test = MatrixTest(testName, matrixConfig);
				TestParams config(
					false,                   //const bool _bVerboseStats,
					numTestRepeatitions,     //const int _numTestRepeatitions,
					false,                   //const bool _bVerboseTest,
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
					<< "\t" << result.GetStandardDeviation()
					<< "\n");
			}
		}
	}
}

void primes(const std::vector<TargetLibrary>& testedLibraries, const int maxNumThreads, const int numTestRepeatitions, const int searchRangeMin, const int searchRangeMax, std::ostream& printBuffer)
{
	const char* testName = "Primes";
	DOUBLE_PRINT("Tested" << testName << " (num retries: " << numTestRepeatitions << ")\n");
	DOUBLE_PRINT("LIB      \tNUM_THR\tSUCC\tAVG_TIM\tSTD_DEV\n");

	for (const TargetLibrary& targetLibrary : testedLibraries)
	{
		for (int numThreads = 1; numThreads <= maxNumThreads; ++numThreads)
		{
			if (numThreads == 1 && targetLibrary != TargetLibrary::NoLibrary) continue;
			if (numThreads != 1 && targetLibrary == TargetLibrary::NoLibrary) continue;

			PrimeTestConfig primeConfig = PrimeTestConfig(searchRangeMin, searchRangeMax);
			PrimeTest test = PrimeTest(testName, primeConfig);
			TestParams config(
				false,               //const bool _bVerboseStats,
				numTestRepeatitions, //const int _numTestRepeatitions,
				true,                //const bool _bVerboseTest,
				numThreads,          //const int _numThreadsToUse,
				ForSchedule::None,   //const ForSchedule _forSchedule,
				0,                   //const int forChunkSize,
				nullptr              //void* _userData
			);

			std::vector<TestResult> perLibraryResults;
			test.PerformTests({ targetLibrary }, config, perLibraryResults);
			const TestResult& result = perLibraryResults[0];
			DOUBLE_PRINT(
				LibraryToString(result.testedLibrary)
				<< "\t" << numThreads
				<< "\t" << (result.DidTestSucceed() ? 1 : 0)
				<< "\t" << result.GetAverageResultTime()
				<< "\t" << result.GetStandardDeviation()
				<< "\n");
		}
	}
}

void string(const std::vector<TargetLibrary>& testedLibraries, const int numTestRepeatitions, const int numStrings, const int stringLengths, std::ostream& printBuffer)
{
	const char* testName = "Strings";
	printf("Tested %s (num retries: %d)\n", testName, numTestRepeatitions);
	printf("LIB      \tSUCC\tAVG_TIM\tSTD_DEV\n");

	for (const TargetLibrary& targetLibrary : testedLibraries)
	{
		StringTestConfig primeConfig = StringTestConfig(numStrings, stringLengths);
		const int numThreads = (targetLibrary == TargetLibrary::NoLibrary) ? 1 : 4;
		StringTest test = StringTest(testName, primeConfig);
		TestParams config(
			false,               //const bool _bVerboseStats,
			numTestRepeatitions, //const int _numTestRepeatitions,
			true,                //const bool _bVerboseTest,
			numThreads,          //const int _numThreadsToUse,
			ForSchedule::None,   //const ForSchedule _forSchedule,
			0,                   //const int forChunkSize,
			nullptr              //void* _userData
		);

		std::vector<TestResult> perLibraryResults;
		test.PerformTests({ targetLibrary }, config, perLibraryResults);
		const TestResult& result = perLibraryResults[0];
		DOUBLE_PRINT(
			LibraryToString(result.testedLibrary)
			<< "\t" << (result.DidTestSucceed() ? 1 : 0)
			<< "\t" << result.GetAverageResultTime()
			<< "\t" << result.GetStandardDeviation()
			<< "\n");
	}
}

#define TimeStamp std::chrono::steady_clock::time_point
#define TimeNow std::chrono::steady_clock::now
#define TimeSpan unsigned long long

int main()
{
	const int globalNumRepeatitions = 20;
	const int globalMaxNumThreads = 8;

	//PrioritySetter::SetPriority(Priority::Realtime);

	std::vector<ForTestVariant> forVariants = BuildForVariants(globalMaxNumThreads, 5); //test up to 8 threads, up to 32 chunks
	std::vector<TargetLibrary> doLibraries = { TargetLibrary::NoLibrary, TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB, TargetLibrary::dlib };
	std::vector<TargetLibrary> sectionsLibraries = { TargetLibrary::NoLibrary, TargetLibrary::OpenMP, TargetLibrary::ParallelLib, TargetLibrary::IntelTBB };

	// TimeStamp app_launched = TimeNow();
	// 
	// std::ofstream mandel_file("mandelbrot_result.txt");
	// mandelbrot(forVariants, globalNumRepeatitions, false /* don't export images */, mandel_file);
	// 
	// std::ofstream matrix_file1("matrix_result.txt");
	// std::ofstream matrix_file2("matrix_result_transposed.txt");
	// std::ofstream matrix_file3("matrix_result_nested.txt");
	// std::ofstream matrix_file4("matrix_result_nested_transposed.txt");
	// matrix(forVariants, globalNumRepeatitions, false, false, matrix_file1);
	// matrix(forVariants, globalNumRepeatitions, true,  false, matrix_file2);
	// matrix(forVariants, globalNumRepeatitions, false, true,  matrix_file3);
	// matrix(forVariants, globalNumRepeatitions, true,  true,  matrix_file4);
	// 
	   std::ofstream primes5000_file("primes_result_5000.txt");
	   std::ofstream primes50000_file("primes_result_50000.txt");
	   std::ofstream str_file("strrr.txt");
	   string({ TargetLibrary::NoLibrary, TargetLibrary::ParallelLib }, globalNumRepeatitions, 10000, 2000, str_file);
	   //primes({ TargetLibrary::NoLibrary, TargetLibrary::ParallelLib }, globalMaxNumThreads, globalNumRepeatitions, 2, 5000, primes5000_file);
	   //primes({ TargetLibrary::NoLibrary, TargetLibrary::ParallelLib }, globalMaxNumThreads, globalNumRepeatitions, 2, 50000, primes50000_file);
	// 
	// std::ofstream string_file1("strings_result_10000strX2000ch.txt");
	// std::ofstream string_file2("strings_result_2000strX10000ch.txt");
	// string(sectionsLibraries, globalNumRepeatitions, 10000, 2000, string_file1);
	// string(sectionsLibraries, globalNumRepeatitions, 2000, 10000, string_file2);
	// 
	// TimeStamp app_finished = TimeNow();
	// TimeSpan total = std::chrono::duration_cast<std::chrono::seconds>(app_finished - app_launched).count();
	// std::cout << "\nall tests done in " << total << "s.\n";
	
getchar();
	return 0;
}
