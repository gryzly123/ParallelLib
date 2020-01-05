#pragma once
class Matrix
{
public:
	double** arr;
	const int rows, cols;

	Matrix(int cols, int rows);
	Matrix(int rows, int cols, int RangeMin, int RangeMax);
	~Matrix();

	void Print();
	void Clear();
	void RandomValues(int RangeMin, int RangeMax);
	
	//static bool Transpose(Matrix& A, Matrix& AT);
	//static bool Multiply(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads);
	//static bool MultiplyTransposed(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads);
	//static bool iMultiply(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads);
	//static bool iMultiplyTransposed(Matrix& A, Matrix& B, Matrix& Result, int TargetNumThreads);
};
