/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

#endif	// traktor_EnterLeave_H
