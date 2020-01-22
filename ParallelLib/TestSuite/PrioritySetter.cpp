#include "stdafx.h"
#include "PrioritySetter.h"

#ifdef __GNUC__ //Linux
#include <sys/time.h>
#include <sys/resource.h>

void PrioritySetter::SetPriority(Priority priority)
{
	switch (priority)
	{
	case Priority::Default:
        //what here	
        break;
    
    case Priority::High:
        //what here
        break;

	case Priority::Realtime:
        setpriority(PRIO_PROCESS, 0, -20);
        break;
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
