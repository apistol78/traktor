#ifndef traktor_amalgam_ReconfiguredAction_H
#define traktor_amalgam_ReconfiguredAction_H

#include "Amalgam/IAction.h"
#include "Amalgam/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Reconfigured action.
 *
 * Applications are notified with this
 * action when any server has been reconfigured.
 */
class T_DLLCLASS ReconfiguredAction : public IAction
{
	T_RTTI_CLASS;

public:
	ReconfiguredAction(int32_t result);

	int32_t getResult() const;

private:
	int32_t m_result;
};

	}
}

#endif	// traktor_amalgam_ReconfiguredAction_H
