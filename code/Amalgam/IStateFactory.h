#ifndef traktor_amalgam_IStateFactory_H
#define traktor_amalgam_IStateFactory_H

#include "Core/Object.h"

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

class IEnvironment;
class IState;

/*! \brief Runtime state factory. */
class T_DLLCLASS IStateFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create state.
	 *
	 * \param environment Runtime environment.
	 * \return State.
	 */
	virtual Ref< IState > create(IEnvironment* environment) const = 0;
};

	}
}

#endif	// traktor_amalgam_IStateFactory_H
