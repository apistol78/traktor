#include "Render/Vulkan/TimeQueryVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TimeQueryVk", TimeQueryVk, ITimeQuery)

TimeQueryVk::TimeQueryVk()
{
}

bool TimeQueryVk::create()
{
	return true;
}

void TimeQueryVk::begin()
{
}

int32_t TimeQueryVk::stamp()
{
	return 0;
}

void TimeQueryVk::end()
{
}

bool TimeQueryVk::ready() const
{
	return false;
}

uint64_t TimeQueryVk::get(int32_t index) const
{
	return 0;
}

	}
}
