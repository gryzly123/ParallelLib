#pragma once

class pSingleton
{
private:

	static pSingleton* ptr;

	pSingleton();

public:

	static const pSingleton& Get();

	const unsigned int NumThreads;

};
