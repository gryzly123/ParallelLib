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
};
