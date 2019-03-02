#include "Render/Ps4/TimeQueryPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TimeQueryPs4", TimeQueryPs4, ITimeQuery)

TimeQueryPs4::TimeQueryPs4(ContextPs4* context)
{
}

bool TimeQueryPs4::create()
{
	return true;
}

void TimeQueryPs4::begin()
{
}

int32_t TimeQueryPs4::stamp()
{
	return 0;
}

void TimeQueryPs4::end()
{
}

bool TimeQueryPs4::ready() const
{
	return false;
}

uint64_t TimeQueryPs4::get(int32_t index) const
{
	return 0;
}

	}
}
