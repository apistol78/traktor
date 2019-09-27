#pragma once

#include "Core/Class/IRuntimeClassFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace svg
	{

/*! \brief SVG class binding factory.
 * \ingroup SVG
 */
class T_DLLCLASS ClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const override final;
};

	}
}

