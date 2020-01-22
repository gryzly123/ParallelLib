#include "stdafx.h"
#include "PrioritySetter.h"

#ifdef __GNUC__ //Linux


void PrioritySetter::SetPriority(Priority priority)
{
	switch (priority)
	{
	case Priority::Default:
	case Priority::High:
	case Priority::Realtime:
	default:
		throw; //not implemented
	}
}

#else //Windows
#include "Windows.h"

void PrioritySetter::SetPriority(Priority priority)
{
	switch (priority)
	{
	case Priority::Default:
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		return;

	case Priority::High:
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		return;

	case Priority::Realtime:
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		return;

	default:
		throw;
	}
}

#endif
