#pragma once

#include <functional>

namespace traktor
{

/*! Automatically call functors when entering and leaving scope.
 * \ingroup Core
 */
class EnterLeave
{
public:
	explicit EnterLeave(const std::function< void() >& enter, const std::function< void() >& leave)
	:	m_leave(leave)
	{
		if (enter)
			enter();
	}

	inline ~EnterLeave()
	{
		if (m_leave)
			m_leave();
	}

private:
	std::function< void() > m_leave;
};

}
