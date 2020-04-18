#include "Singleton.h"
#include <thread>

pSingleton* pSingleton::ptr;

pSingleton::pSingleton()
	: NumThreads(std::thread::hardware_concurrency())
{ }

const pSingleton& pSingleton::Get()
{
	if (ptr == nullptr) ptr = new pSingleton;
	return *ptr;
}
