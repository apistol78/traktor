#ifndef traktor_amalgam_GameClassFactory_H
#define traktor_amalgam_GameClassFactory_H

#include "Core/Class/IRuntimeClassFactory.h"

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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS GameClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_GameClassFactory_H
