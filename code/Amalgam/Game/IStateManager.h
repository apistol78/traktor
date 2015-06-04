#ifndef traktor_amalgam_IStateManager_H
#define traktor_amalgam_IStateManager_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IState;

/*! \brief State manager.
 * \ingroup Amalgam
 *
 * The state manager interface, it provide
 * an interface for user applications to
 * enter another state.
 */
class T_DLLCLASS IStateManager : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Enter state transition.
	 *
	 * \param state New state.
	 */
	virtual void enter(IState* state) = 0;
};

	}
}

#endif	// traktor_amalgam_IStateManager_H
