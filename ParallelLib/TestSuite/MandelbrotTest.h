#pragma once
#include "Test.h"

struct MandelbrotConfig
{
	//screen size
	const int iXmax = 2000;
	const int iYmax = 2000;
	const int MaxColorComponentValue = 255;

	//world ( double) coordinate = parameter plane
	const double CxMin = -2.5;
	const double CxMax = 1.5;
	const double CyMin = -2.0;
	const double CyMax = 2.0;
	const double EscapeRadius = 2.0f;

	const int IterationMax = 200;
};

class MandelbrotTest : public Test
{
public:
	MandelbrotTest(const std::string& inName);
	~MandelbrotTest();

protected:
	MandelbrotConfig config;

	virtual void DoSequentially(const TestParams& In, RetryResult& Out) override;
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out) override;
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out) override;
	virtual void DoBoost(const TestParams& In, RetryResult& Out) override;
	virtual void DoTBB(const TestParams& In, RetryResult& Out) override;
	virtual void DoDlib(const TestParams& In, RetryResult& Out) override;
};
