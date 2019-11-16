#pragma once
#include <vector>
#include <chrono>

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
	TaskEndedSuccessfully = 4,
	TaskFailed = 5
};

//note: I could use typedef here, but C++'s strong typing would not
//allow to perform std::chrono operations on the custom types anymore
#define TimeStamp std::chrono::steady_clock::time_point
#define TimeSpan long long
struct RetryResult
{
private:
	TestPhase testState;
	TimeStamp start;
	TimeSpan phaseTime[3];

public:
	void* userData;

	RetryResult();
	void BeginResourceInit();
	void BeginParallWorkload();
	void BeginResourceCleanup();
	void EndTask(bool bSucceeded);

	inline const TimeSpan& GetResourceInitDuration() const { return phaseTime[0]; }
	inline const TimeSpan& GetParallelWorkloadDuration() const { return phaseTime[1]; }
	inline const TimeSpan& GetResourceCleanupDuration() const { return phaseTime[3]; }
	inline const bool& GetTaskSucceeded() const { return (testState == TestPhase::TaskEndedSuccessfully); }
};

struct TestResult
{
	TargetLibrary testedLibrary;
	std::vector<RetryResult> perTryResults;
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
	const std::string name;
public:
	Test();
	virtual ~Test();

	void PerformTests(std::vector<TargetLibrary> targetLibs, const TestParams& inParams, std::vector<TestResult>& results);
	
	inline TestType GetTestType() const { return type; }
	inline const std::string& GetTestName() const { return name; }

protected:
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out);
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out);
	virtual void DoBoost(const TestParams& In, RetryResult& Out);
};
