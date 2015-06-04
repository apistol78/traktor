#ifndef traktor_amalgam_IEvent_H
#define traktor_amalgam_IEvent_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Event message interface.
 * \ingroup Amalgam
 *
 * Events are system messages, such as reconfiguration, activation etc,
 * which are sent to current state.
 */
class T_DLLCLASS IEvent : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_amalgam_IEvent_H
