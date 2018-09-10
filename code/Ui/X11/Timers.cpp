#include "Core/Assert.h"
#include "Core/Log/Log.h"
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
    int32_t id = m_nid++;
    T_FATAL_ASSERT(m_timers.find(id) == m_timers.end());
    
    Timer& t = m_timers[id];
    t.interval = interval;
    t.until = interval;
    t.fn = fn;

    return id;
}

void Timers::unbind(int32_t id)
{
    size_t nr = m_timers.erase(id);
    T_FATAL_ASSERT(nr > 0);
    m_nid++;
}

void Timers::queue(const std::function< void() >& fn)
{
    m_events.push_back(fn);
}

void Timers::dequeue()
{
    m_events.clear();
}

void Timers::update(int32_t ms)
{
    std::vector< std::function< void(int32_t) > > fns;
    for (auto& it : m_timers)
    {
        Timer& t = it.second;
        if ((t.until -= ms) <= 0)
        {
            t.until = t.interval;
            fns.push_back(t.fn);
        }
    }

    int32_t nid = m_nid;
    for (auto fn : fns)
    {
        fn(0);
        if (nid != m_nid)
            break;
    }

    for (auto it : m_events)
        it();

    m_events.resize(0);
}

Timers::Timers()
:   m_nid(1)
{
}

    }
}