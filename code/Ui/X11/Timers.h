#pragma once

#include <functional>
#include <map>
#include <vector>

namespace traktor
{
    namespace ui
    {

class Timers
{
public:
    static Timers& getInstance();

    int32_t bind(int32_t interval, const std::function< void(int32_t) >& fn);

    void unbind(int32_t id);

    void queue(const std::function< void() >& fn);

    void dequeue();

    void update();

private:
    std::map< int32_t, std::function< void(int32_t) > > m_timers;
    std::vector< std::function< void() > > m_events;
    int32_t m_nid;

    Timers();
};

    }
}
