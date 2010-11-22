#ifndef traktor_amalgam_IAction_H
#define traktor_amalgam_IAction_H

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

/*! \brief Action message interface. */
class T_DLLCLASS IAction : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_amalgam_IAction_H
