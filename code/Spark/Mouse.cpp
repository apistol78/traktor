#include <algorithm>
#include "Core/Class/CastAny.h"
#include "Spark/Mouse.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Mouse", Mouse, Object)

void Mouse::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	m_eventButtonDown.issue();
}

void Mouse::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	m_eventButtonUp.issue();
}

void Mouse::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	m_eventMouseMove.issue();
}

void Mouse::eventMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	Any argv = CastAny< int >::set(delta);
	m_eventMouseWheel.issue(1, &argv);
}

	}
}
