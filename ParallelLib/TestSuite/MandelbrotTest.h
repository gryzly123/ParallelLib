#pragma once
#include "Test.h"
#include <string>

struct MandelbrotTestConfig
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
	const bool bExportImages = false;

	MandelbrotTestConfig(bool bExportImages) : bExportImages(bExportImages) { }
};

class MandelbrotTest : public Test
{
public:
	MandelbrotTest(const std::string& inName, MandelbrotTestConfig& testConfig);
	~MandelbrotTest();

protected:
	MandelbrotTestConfig testConfig;

	virtual void DoSequentially(const TestParams& In, RetryResult& Out) override;
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out) override;
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out) override;
	virtual void DoTBB(const TestParams& In, RetryResult& Out) override;
	virtual void DoDlib(const TestParams& In, RetryResult& Out) override;

private:
	std::string BuildFilename(const TestParams& In);
};
