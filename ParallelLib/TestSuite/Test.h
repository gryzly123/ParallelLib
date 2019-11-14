#pragma once
#include <vector>

enum class ForSchedule : unsigned char
{
	Static = 0,
	Dynamic = 1,
	Guided = 2
};

enum class TargetLibrary : unsigned char
{
	OpenMP = 1,
	ParallelLib = 2,
	Boost = 4
};


struct TestParams
{
	//Test() method params
	const bool bVerboseStats; //print task progress and time stats
	const int numTestRepeatitions; //how many times to test a single library

	//actual test method params
	const bool bVerboseTest; //print task progress and time stats
	const int numThreadsToUse; //how many threads to use in the algorithm
	const ForSchedule forSchedule; //which for schedule to use in for-based parallelism
	void* userData; //additional test-class-specific helper data

	TestParams(
		const bool _bVerboseStats,
		const int _numTestRepeatitions,
		const bool _bVerboseTest,
		const int _numThreadsToUse,
		const ForSchedule _forSchedule,
		void* _userData);
};

enum class TestPhase : unsigned char
{
	InitializedTestCase = 0,
	BegunResourceInitialization = 1,
	BegunParallelWorkload = 2,
	BegunResourceCleanup = 3,
	TaskEndedSuccessfully = 4
};


struct TestResult
{
	TargetLibrary testedLibrary;
	TestPhase testPhase;
	std::vector<long long> perTryResult;
	void* userData; //additional test-class-specific helper data

	int GetNumTestRepeatitions();
	int GetAverageResultTime(); //returns mean average of all test times
	bool DidTestFail(); //test is considered failed if the last testPhase is not a successful task end

	TestResult(const TargetLibrary _testedLibrary);
};

enum class TestType : unsigned char
{
	None                  = 0,
	MatrixMultiplication  = 1,
	Mandelbrot            = 2,
	ImageFilters          = 3,
	Sierpinski            = 4
};


class Test
{
private:
	const TestType type;

public:

	Test();
	virtual ~Test();

	void PerformTests(std::vector<TargetLibrary> targetLibs, const TestParams& inParams, std::vector<TestResult>& results);

protected:
	virtual void DoParallelLib(const TestParams& In, TestResult& Out);
	virtual void DoOpenMP(const TestParams& In, TestResult& Out);
	virtual void DoBoost(const TestParams& In, TestResult& Out);
	TestType GetTestType();
};

