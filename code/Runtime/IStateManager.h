#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

class IState;

/*! State manager.
 * \ingroup Runtime
 *
 * The state manager interface, it provide
 * an interface for user applications to
 * enter another state.
 */
class T_DLLCLASS IStateManager : public Object
{
	T_RTTI_CLASS;

public:
	/*! Enter state transition.
	 *
	 * \param state New state.
	 */
	virtual void enter(IState* state) = 0;
};

	}
}

