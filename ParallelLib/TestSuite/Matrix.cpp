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
