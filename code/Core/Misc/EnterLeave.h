#ifndef traktor_EnterLeave_H
#define traktor_EnterLeave_H

#include "Core/Functor/Functor.h"

namespace traktor
{

/*! \brief Automatically call functors when entering and leaving scope.
 * \ingroup Core
 */
class EnterLeave
{
public:
	inline EnterLeave(Functor* enter, Functor* leave)
	:	m_leave(leave)
	{
		if (enter)
		{
			(*enter)();
			delete enter;
		}
	}

	inline ~EnterLeave()
	{
		if (m_leave)
		{
			(*m_leave)();
			delete m_leave;
		}
	}

private:
	Functor* m_leave;
};

}

#endif	// traktor_EnterLeave_H
