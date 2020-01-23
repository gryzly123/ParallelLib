#include "stdafx.h"
#include "PrioritySetter.h"

#ifdef __GNUC__ //Linux
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

void PrioritySetter::SetPriority(Priority priority)
{
    int errcode;
	switch (priority)
	{
	case Priority::Default:
        errcode = setpriority(PRIO_PROCESS, getpid(), 0);
        break;
    
    case Priority::High:
        errcode = setpriority(PRIO_PROCESS, getpid(), -10);
        break;

	case Priority::Realtime:
        errcode = setpriority(PRIO_PROCESS, getpid(), -20);
        break;
	default:
		throw; //not implemented
	}

    if(errcode != 0)
    {
        printf("setpriority() didn't succeed (errno %d).\nAre you running with sudo? Exiting\n", errcode);
        throw;
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
