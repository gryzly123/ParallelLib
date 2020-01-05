#pragma once
#include "Test.h"
class Matrix;

struct MatrixTestConfig
{
	const int matrixSize_AX = 300;
	const int matrixSize_AY = 500;
	const int matrixSize_BX = 400;
	const int matrixSize_BY = 300;
	const bool bTransposeMatrix = false;
	const bool bDoubleForParallelization = false;

	MatrixTestConfig();
	MatrixTestConfig(bool bTransposeMatrix, bool bDoubleForParallelization);
};

class MatrixTest : public Test
{
public:
	MatrixTest(const std::string& inName, const MatrixTestConfig& config);
	~MatrixTest();

private:
	MatrixTestConfig testConfig;
	Matrix* A;
	Matrix* B;
	Matrix* Result;

protected:
	virtual void DoSequentially(const TestParams& In, RetryResult& Out) override;
	virtual void DoParallelLib(const TestParams& In, RetryResult& Out) override;
	virtual void DoOpenMP(const TestParams& In, RetryResult& Out) override;
	//virtual void DoBoost(const TestParams& In, RetryResult& Out) override;
	virtual void DoTBB(const TestParams& In, RetryResult& Out) override;
	virtual void DoDlib(const TestParams& In, RetryResult& Out) override;
};

