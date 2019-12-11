#pragma once

#include "Core/Functor/Functor.h"

namespace traktor
{

/*! Automatically call functors when entering and leaving scope.
 * \ingroup Core
 */
class EnterLeave
{
public:
	inline EnterLeave(Functor* enter, Functor* leave)
	:	m_leave(leave)
	{
		if (enter)
			(*enter)();
	}

	inline ~EnterLeave()
	{
		if (m_leave)
			(*m_leave)();
	}

private:
	Ref< Functor > m_leave;
};

}

