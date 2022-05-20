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

	int32_t bind(int32_t interval, const std::function< void() >& fn);

	void unbind(int32_t id);

	void update(double s);

private:
	struct Timer
	{
		int32_t interval;
		double until;
		std::function< void() > fn;
	};

	std::map< int32_t, Timer > m_timers;
	int32_t m_nid = 1;
};

	}
}
