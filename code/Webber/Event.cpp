#include "Webber/Event.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Event", Event, Object)

void Event::raise()
{
	for (auto fn : m_fns)
		fn();
}

void Event::operator += (const std::function< void() >& fn)
{
	m_fns.push_back(fn);
}

	}
}
