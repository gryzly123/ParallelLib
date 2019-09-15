#pragma once
#include "SetOnce.h"

enum class pSchedule : unsigned char
{
	Static = 0,
	Dynamic = 1,
	Guided = 2
};

class ISchedulable
{
protected:
	pSetOnce<pSchedule> ScheduleType;
};
