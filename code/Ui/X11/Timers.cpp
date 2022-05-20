#include "Core/Assert.h"
#include "Core/Containers/AlignedVector.h"
#include "Ui/X11/Timers.h"

namespace traktor
{
	namespace ui
	{

Timers& Timers::getInstance()
{
	static Timers s_instance;
	return s_instance;
}

int32_t Timers::bind(int32_t interval, const std::function< void() >& fn)
{
	int32_t id = m_nid++;
	T_FATAL_ASSERT(m_timers.find(id) == m_timers.end());

	Timer& t = m_timers[id];
	t.interval = interval;
	t.until = interval / 1000.0;
	t.fn = fn;

	return id;
}

void Timers::unbind(int32_t id)
{
	size_t nr = m_timers.erase(id);
	T_FATAL_ASSERT(nr > 0);
	m_nid++;
}

void Timers::update(double s)
{
	AlignedVector< std::function< void() > > fns;
	for (auto& it : m_timers)
	{
		Timer& t = it.second;
		if ((t.until -= s) <= 0.0)
		{
			t.until = t.interval / 1000.0;
			fns.push_back(t.fn);
		}
	}
	for (auto fn : fns)
		fn();
}

	}
}
