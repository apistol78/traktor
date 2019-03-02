#include "Core/Thread/Event.h"

namespace traktor
{

Event::Event()
{
	m_handle = 0;
}

Event::~Event()
{
}

void Event::pulse(int count)
{
}

void Event::broadcast()
{
}

void Event::reset()
{
}

bool Event::wait(int timeout)
{
	return true;
}

}
