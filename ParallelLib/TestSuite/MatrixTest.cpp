#include "stdafx.h"
#include "MatrixTest.h"
#include "Matrix.h"

MatrixTestConfig::MatrixTestConfig() { }

MatrixTestConfig::MatrixTestConfig(bool bTransposeMatrix, bool bDoubleForParallelization)
	: bTransposeMatrix(bTransposeMatrix)
	, bDoubleForParallelization(bDoubleForParallelization)
{ }

MatrixTest::MatrixTest(const std::string& inName, const MatrixTestConfig& config)
	: Test(TestType::MatrixMultiplication, inName)
	, testConfig(config)
	, A(new Matrix(config.matrixSize_AX, config.matrixSize_AY))
	, B(config.bTransposeMatrix ? new Matrix(config.matrixSize_BY, config.matrixSize_BX) : new Matrix(config.matrixSize_BX, config.matrixSize_BY))
	, Result(new Matrix(config.matrixSize_BX, config.matrixSize_AY))
{

}

MatrixTest::~MatrixTest()
{
	delete A;
	delete B;
	delete Result;
}

#pragma optimize("", off)
void MatrixTest::DoSequentially(const TestParams& In, RetryResult& Out)
{
	Out.BeginResourceInit();

	if (!testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->cols || A->cols != B->rows))
	{
		throw;
	}
	if ( testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->rows || A->cols != B->cols))
	{
		throw;
	}

	const int kmax = A->cols;
	Result->Clear();

	Out.BeginParallelWorkload();
	if (!testConfig.bTransposeMatrix)
	{
		for (int i = 0; i < Result->rows; ++i)
			for (int j = 0; j < Result->cols; ++j)
				for (int k = 0; k < kmax; ++k)
					Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
	}
	else
	{
		for (int i = 0; i < Result->rows; ++i)
			for (int j = 0; j < Result->cols; ++j)
				for (int k = 0; k < kmax; ++k)
					Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
	}

	Out.BeginResourceCleanup();
	Out.EndTask(true);
}

#include "ParallelLib/ParallelLib.h"
void MatrixTest::DoParallelLib(const TestParams & In, RetryResult & Out)
{
	Out.BeginResourceInit();

	if (!testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->cols || A->cols != B->rows))
	{
		throw;
	}
	if (testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->rows || A->cols != B->cols))
	{
		throw;
	}

	const int kmax = A->cols;
	Result->Clear();

	Out.BeginParallelWorkload();

	if (!testConfig.bDoubleForParallelization)
	{
		if (!testConfig.bTransposeMatrix)
		{
			parallel_for(loop, i, 0, Result->rows, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
			{
				for (int j = 0; j < Result->cols; ++j)
					for (int k = 0; k < kmax; ++k)
						Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			});
		}
		else
		{
			parallel_for(loop, i, 0, Result->rows, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
			{
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			});
		}
	}
	else
	{
		if (!testConfig.bTransposeMatrix)
		{
			parallel_for(loop, i, 0, Result->rows, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
			{
				parallel_for(inner, j, 0, Result->cols, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
				{
					for (int k = 0; k < kmax; ++k)
						Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
				});
			});
		}
		else
		{
			parallel_for(loop, i, 0, Result->rows, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
			{
				parallel_for(inner, j, 0, Result->cols, 1, num_threads(In.numThreadsToUse) schedulev((pSchedule)In.forSchedule, In.forChunkSize),
				{
					for (int k = 0; k < kmax; ++k)
						Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
				});
			});
		}
	}

	Out.BeginResourceCleanup();
	Out.EndTask(true);
}
#undef schedule
#undef num_threads

void MatrixTest::DoOpenMP(const TestParams & In, RetryResult & Out)
{
	Out.BeginResourceInit();

	if (!testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->cols || A->cols != B->rows))
	{
		throw;
	}
	if (testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->rows || A->cols != B->cols))
	{
		throw;
	}

	const int kmax = A->cols;
	Result->Clear();

	Out.BeginParallelWorkload();

	if (!testConfig.bDoubleForParallelization)
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;

		case ForSchedule::Guided:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;
		}
	}
	else
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(static, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(dynamic, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;

		case ForSchedule::Guided:
			if (!testConfig.bTransposeMatrix)
			{
				#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
			}
			else
			{
				#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
				for (int i = 0; i < Result->rows; ++i)
					#pragma omp parallel for schedule(guided, In.forChunkSize) num_threads(In.numThreadsToUse)
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
			}
			break;
		}
	}

	Out.BeginResourceCleanup();
	Out.EndTask(true);
}

#undef parallel_for
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
void MatrixTest::DoTBB(const TestParams & In, RetryResult & Out)
{
	Out.BeginResourceInit();

	if (!testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->cols || A->cols != B->rows))
	{
		throw;
	}
	if (testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->rows || A->cols != B->cols))
	{
		throw;
	}

	const int kmax = A->cols;
	Result->Clear();

	Out.BeginParallelWorkload();

	if (!testConfig.bDoubleForParallelization)
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			if (!testConfig.bTransposeMatrix)
			{
				tbb::parallel_for(0, Result->rows, [&](size_t i)
					{
						for (int j = 0; j < Result->cols; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
					}, tbb::static_partitioner());
			}
			else
			{
				tbb::parallel_for(0, Result->rows, [&](size_t i)
					{
						for (int j = 0; j < Result->cols; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
					}, tbb::static_partitioner());
			}
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				tbb::parallel_for(0, Result->rows, [&](size_t i)
				{
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
				});
			}
			else
			{
				tbb::parallel_for(0, Result->rows, [&](size_t i)
				{
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
				});
			}
			break;

		case ForSchedule::Guided:
			throw;
		}
	}
	else
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			if (!testConfig.bTransposeMatrix)
			{
				tbb::parallel_for(tbb::blocked_range2d<int, int>(0, Result->rows, 0, Result->cols), [&](const tbb::blocked_range2d<int, int> &range)
				{
					const int i_end = range.rows().end();
					const int j_end = range.cols().end();
					for (int i = range.rows().begin(); i < i_end; ++i)
						for (int j = range.cols().begin(); j < j_end; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
				}, tbb::static_partitioner());
			}
			else
			{
				tbb::parallel_for(tbb::blocked_range2d<int, int>(0, Result->rows, 0, Result->cols), [&](const tbb::blocked_range2d<int, int> &range)
				{
					const int i_end = range.rows().end();
					const int j_end = range.cols().end();
					for (int i = range.rows().begin(); i < i_end; ++i)
						for (int j = range.cols().begin(); j < j_end; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
				}, tbb::static_partitioner());
			}
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				tbb::parallel_for(tbb::blocked_range2d<int, int>(0, Result->rows, 0, Result->cols), [&](const tbb::blocked_range2d<int, int> &range)
				{
					const int i_end = range.rows().end();
					const int j_end = range.cols().end();
					for (int i = range.rows().begin(); i < i_end; ++i)
						for (int j = range.cols().begin(); j < j_end; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
				});
			}
			else
			{
				tbb::parallel_for(tbb::blocked_range2d<int, int>(0, Result->rows, 0, Result->cols), [&](const tbb::blocked_range2d<int, int> &range)
				{
					const int i_end = range.rows().end();
					const int j_end = range.cols().end();
					for (int i = range.rows().begin(); i < i_end; ++i)
						for (int j = range.cols().begin(); j < j_end; ++j)
							for (int k = 0; k < kmax; ++k)
								Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
				});
			}
			break;

		case ForSchedule::Guided:
			throw;
		}
	}

	Out.BeginResourceCleanup();
	Out.EndTask(true);

}

#undef TimeStamp
#include "dlib/threads/parallel_for_extension.h"
void MatrixTest::DoDlib(const TestParams & In, RetryResult & Out)
{
	Out.BeginResourceInit();

	if (!testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->cols || A->cols != B->rows))
	{
		throw;
	}
	if ( testConfig.bTransposeMatrix && (Result->rows != A->rows || Result->cols != B->rows || A->cols != B->cols))
	{
		throw;
	}

	const int kmax = A->cols;
	Result->Clear();

	dlib::thread_pool tpool(In.numThreadsToUse);

	Out.BeginParallelWorkload();

	if (!testConfig.bDoubleForParallelization)
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			throw; //not available
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				dlib::parallel_for(tpool, 0, Result->rows, [&](long i)
				{
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
				}, In.forChunkSize);
			}
			else
			{
				dlib::parallel_for(tpool, 0, Result->rows, [&](long i)
				{
					for (int j = 0; j < Result->cols; ++j)
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
				}, In.forChunkSize);
			}
			break;

		case ForSchedule::Guided:
			throw; //not available
		}
	}
	else
	{
		switch (In.forSchedule)
		{
		case ForSchedule::Static:
			throw; //not available
			break;

		case ForSchedule::Dynamic:
			if (!testConfig.bTransposeMatrix)
			{
				dlib::parallel_for(tpool, 0, Result->rows, [&](long i)
				{
					dlib::parallel_for(tpool, 0, Result->cols, [&](long j)
					{
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[k][j];
					});
				}, In.forChunkSize);
			}
			else
			{
				dlib::parallel_for(tpool, 0, Result->rows, [&](long i)
				{
					dlib::parallel_for(tpool, 0, Result->cols, [&](long j)
					{
						for (int k = 0; k < kmax; ++k)
							Result->arr[i][j] += A->arr[i][k] * B->arr[j][k];
					});
				}, In.forChunkSize);
			}
			break;

		case ForSchedule::Guided:
			throw; //not available
		}
	}

	Out.BeginResourceCleanup();
	Out.EndTask(true);
}
