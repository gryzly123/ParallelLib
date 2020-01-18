#pragma once
#include <vector>
#include <chrono>

// ---------------------- TYPEDEFS --------------------

//note: I could use actual typedef here, but C++'s strong typing would
//not allow to perform std::chrono operations on the custom types anymore
#define TimeStamp std::chrono::steady_clock::time_point
#define TimeSpan unsigned long long

// ----------------------- ENUMS -----------------------

//which schedule should be used in for-based parallel tasks
enum class ForSchedule : unsigned char
{
	Static = 0,
	Dynamic = 1,
	Guided = 2,
	None = 255
};
const char* ForScheduleToString(ForSchedule Schedule); //ToString method for this enum

//tested library
enum class TargetLibrary : unsigned char
{
	NoLibrary = 1, //do sequentially
	OpenMP = 2,
	ParallelLib = 4,
	IntelTBB = 8,
	dlib = 16
};
const char* LibraryToString(TargetLibrary Library); //ToString method for this enum

//test phase of the currently running test instance ("retry")
enum class TestPhase : unsigned char
{
	InitializedTestCase = 0,
	BegunResourceInitialization = 1,
	BegunParallelWorkload = 2,
	BegunResourceCleanup = 3,
	TaskEndedSuccessfully = 4,
	TaskFailed = 5
};

//test class ID
enum class TestType : unsigned char
{
	None = 0,
	MatrixMultiplication = 1,
	Mandelbrot = 2,
	PrimeNumbers = 3,
	StringTest = 4
};

// ----------------------- STRUCTS -----------------------

//input parameters for the test
struct TestParams
{
	//Test() method params
	const bool bVerboseStats; //print task progress and time stats
	const int numTestRepeatitions; //how many times to test a single library

	//actual test method params
	const bool bVerboseTest; //print task progress and time stats
	const int numThreadsToUse; //how many threads to use in the algorithm
	const ForSchedule forSchedule; //which for schedule to use in for-based parallelism
	const int forChunkSize; //size of chunks for static and dynamic for loops
	void* userData; //additional test-class-specific helper data

	TestParams(
		const bool _bVerboseStats,
		const int _numTestRepeatitions,
		const bool _bVerboseTest,
		const int _numThreadsToUse,
		const ForSchedule _forSchedule,
		const int _forChunkSize,
		void* _userData);
};

//single test retry result
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
	void BeginParallelWorkload();
	void BeginResourceCleanup();
	void EndTask(bool bSucceeded);

	inline const TimeSpan& GetResourceInitDuration() const { return phaseTime[0]; }
	inline const TimeSpan& GetParallelWorkloadDuration() const { return phaseTime[1]; }
	inline const TimeSpan& GetResourceCleanupDuration() const { return phaseTime[2]; }
	inline const bool GetTaskSucceeded() const { return (testState == TestPhase::TaskEndedSuccessfully); }
};

//full test result, includes all test retries
struct TestResult
{
	TargetLibrary testedLibrary;
	std::vector<RetryResult> perTryResults;
	void* userData; //additional test-class-specific helper data

	size_t GetNumTestRepeatitions() const;
	TimeSpan GetAverageResultTime() const; //returns mean average of all test times
	bool DidTestSucceed() const; //test is considered failed if the last testPhase is not a successful task end

	TestResult(const TargetLibrary _testedLibrary);
};

// --------------------- TEST CLASS ---------------------

class Test
{
private:
	const TestType type;
	const std::string name;
	int testNum;
	TargetLibrary runningLibrary;

	virtual void DoSequentially(const TestParams& In, RetryResult& Out);
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out);
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out);
	virtual void DoTBB(const TestParams& In, RetryResult& Out);
	virtual void DoDlib(const TestParams& In, RetryResult& Out);

public:
	Test();
	virtual ~Test();

	void PerformTests(std::vector<TargetLibrary> targetLibs, const TestParams& inParams, std::vector<TestResult>& results);
	
	inline TestType GetTestType() const { return type; }
	inline const std::string& GetTestName() const { return name; }

protected:
	Test(TestType inType, const std::string& inName);

	inline const int& GetTestNum() const  { return testNum; }
	inline const TargetLibrary& GetRunningLibrary() const  { return runningLibrary; }
};

#ifndef IN_TEST_CPP
	#undef TimeStamp
	#undef TimeSpan
#endif
