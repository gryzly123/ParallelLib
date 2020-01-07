#pragma once
#include "Test.h"

struct PrimeTestConfig
{
	const int searchRangeMin = 1;
	const int searchRangeMax = 500000;
	PrimeTestConfig();
	PrimeTestConfig(int searchRangeMin, int searchRangeMax);
};

class PrimeTest : public Test
{
public:
	PrimeTest(const std::string& name, PrimeTestConfig& testConfig);

	PrimeTestConfig testConfig;

protected:

	bool IsPrime(int index);

	virtual void DoSequentially(const TestParams& In, RetryResult& Out) override;
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out) override;
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out) override;
	//virtual void DoBoost(const TestParams& In, RetryResult& Out) override;
	//virtual void DoTBB(const TestParams& In, RetryResult& Out) override;
	//virtual void DoDlib(const TestParams& In, RetryResult& Out) override;
};

