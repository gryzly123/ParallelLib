#pragma once

template<class T>
class pSetOnce
{
private:
	T object;
	bool bWasSet = false;

public:

	inline void Set(const T& Data);

	inline bool OptionalSet(const T& Data);

	inline const T& Get();

	inline bool WasSet();
};

template<class T>
void pSetOnce<T>::Set(const T& Data)
{
	if (bWasSet) throw;
	object = Data;
	bWasSet = true;
}

template<class T>
bool pSetOnce<T>::OptionalSet(const T& Data)
{
	if (bWasSet) return false;
	object = Data;
	bWasSet = true;
	return true;
}

template<class T>
const T& pSetOnce<T>::Get()
{
	return object;
}

template<class T>
bool pSetOnce<T>::WasSet()
{
	return bWasSet;
}
