#include "Core/Assert.h"
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

int32_t Timers::bind(int32_t interval, const std::function< void(int32_t) >& fn)
{
    T_FATAL_ASSERT(!m_inupdate);
    m_timers[m_nid++] = fn;
}

void Timers::unbind(int32_t id)
{
    T_FATAL_ASSERT(!m_inupdate);
    m_timers.erase(id);
}

void Timers::update()
{
    m_inupdate = true;
    for (auto it : m_timers)
        it.second(it.first);
    m_inupdate = false;
}

Timers::Timers()
:   m_nid(1)
,   m_inupdate(false)
{
}

    }
}