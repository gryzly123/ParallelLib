#pragma once

enum class Priority : unsigned char
{
	Default = 0,
	High = 1,
	Realtime = 2
};

namespace PrioritySetter
{
	void SetPriority(Priority priority);
};
