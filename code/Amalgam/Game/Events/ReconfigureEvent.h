#ifndef traktor_amalgam_ReconfigureEvent_H
#define traktor_amalgam_ReconfigureEvent_H

#include "Amalgam/Game/Types.h"
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

/*! \brief Reconfigure event.
 * \ingroup Amalgam
 *
 * Applications are notified with this
 * event when any server has been reconfigured.
 */
class T_DLLCLASS ReconfigureEvent : public Object
{
	T_RTTI_CLASS;

public:
	ReconfigureEvent(bool finished, int32_t result);

	bool isFinished() const;

	int32_t getResult() const;

private:
	bool m_finished;
	int32_t m_result;
};

	}
}

#endif	// traktor_amalgam_ReconfigureEvent_H
