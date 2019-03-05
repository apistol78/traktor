#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS IRuntimePlugin : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(IEnvironment* environment) = 0;

	virtual void destroy(IEnvironment* environment) = 0;

	virtual Ref< IState > createInitialState(IEnvironment* environment) = 0;
};

	}
}

