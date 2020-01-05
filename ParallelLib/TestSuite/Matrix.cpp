#include "stdafx.h"
#include "Matrix.h"

Matrix::Matrix(int cols, int rows) : rows(rows), cols(cols)
{
    arr = new double*[rows];
    for (int i = 0; i < rows; ++i)
    {
        arr[i] = new double[cols];
        for (int j = 0; j < cols; ++j)
            arr[i][j] = 0;
    }
}

Matrix::~Matrix()
{
    for (int i = 0; i < rows; ++i)
        delete[] arr[i];
    delete[] arr;
}

void Matrix::Clear()
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            arr[i][j] = 0;
}

Matrix::Matrix(int rows, int cols, int RangeMin, int RangeMax) : rows(rows), cols(cols)
{
    arr = new double*[rows];
    for (int i = 0; i < rows; ++i) arr[i] = new double[cols];
    RandomValues(RangeMin, RangeMax);
}

void Matrix::RandomValues(int RangeMin, int RangeMax)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            arr[i][j] = RangeMin + (rand() % RangeMax);
}

void Matrix::Print()
{
    for (int j = 0; j < rows; ++j)
    {
        for (int i = 0; i < cols; ++i)
            std::cout << arr[j][i] << "\t";
        std::cout << "\n";
    }
}
/*
bool Matrix::Multiply(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads)
{
	int num_multi;
    if (Result.rows != A.rows
    || Result.cols != B.cols
    || A.cols != B.rows) return false;
    int kmax = A.cols;

    if (TargetNumThreads == 1)
    {
        for (int i = 0; i < Result.rows; ++i)
            for (int j = 0; j < Result.cols; ++j)
				for (int k = 0; k < kmax; ++k)
				{
                    Result.arr[i][j] += A.arr[i][k] * B.arr[k][j];
					++num_multi;
				}
    }
    else
    {
        omp_set_num_threads(TargetNumThreads);
        #pragma omp parallel for
        for (int i = 0; i < Result.rows; ++i)
            #pragma omp parallel for
            for (int j = 0; j < Result.cols; ++j)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[k][j];
    }
	std::cout << num_multi; getchar();
    return true;
}
bool Matrix::iMultiply(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads)
{
    if (Result.rows != A.rows || Result.cols != B.cols || A.cols != B.rows) return false;
    int kmax = A.cols;

    if (TargetNumThreads == 1)
    {
        for (int j = 0; j < Result.cols; ++j)
            for (int i = 0; i < Result.rows; ++i)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[k][j];
    }
    else
    {
        omp_set_num_threads(TargetNumThreads);
        #pragma omp parallel for
        for (int j = 0; j < Result.cols; ++j)
            #pragma omp parallel for
            for (int i = 0; i < Result.rows; ++i)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[k][j];
    }
    return true;
}

bool Matrix::MultiplyTransposed(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads)
{
    if (Result.rows != A.rows || Result.cols != B.rows || A.cols != B.cols) return false;
    int kmax = A.cols;

    if (TargetNumThreads == 1)
    {
    for (int i = 0; i < Result.rows; ++i)
        for (int j = 0; j < Result.cols; ++j)
            for (int k = 0; k < kmax; ++k)
                Result.arr[i][j] += A.arr[i][k] * B.arr[j][k];
    }
    else
    {
        omp_set_num_threads(TargetNumThreads);
        #pragma omp parallel for
        for (int i = 0; i < Result.rows; ++i)
            #pragma omp parallel for
            for (int j = 0; j < Result.cols; ++j)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[j][k];
    }
    return true;
}

bool Matrix::iMultiplyTransposed(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads)
{
    if (Result.rows != A.rows || Result.cols != B.rows || A.cols != B.cols) return false;
    int kmax = A.cols;

    if (TargetNumThreads == 1)
    {
        for (int j = 0; j < Result.cols; ++j)
            for (int i = 0; i < Result.rows; ++i)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[j][k];
    }
    else
    {
        omp_set_num_threads(TargetNumThreads);
        #pragma omp parallel for
        for (int j = 0; j < Result.cols; ++j)
            #pragma omp parallel for
            for (int i = 0; i < Result.rows; ++i)
                for (int k = 0; k < kmax; ++k)
                    Result.arr[i][j] += A.arr[i][k] * B.arr[j][k];
    }
    return true;
}

bool Matrix::Transpose(Matrix& A, Matrix& AT)
{
    if (A.rows != AT.cols || A.cols != AT.rows) return false;
    for (int j = 0; j < AT.rows; ++j)
        for (int i = 0; i < AT.cols; ++i)
            AT.arr[j][i] = A.arr[i][j];
    return true;
}
*/