#pragma once

#include "Resource/ResourceHandle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! Explicit resource handle.
 * \ingroup Resource
 */
class T_DLLCLASS ExplicitResourceHandle : public ResourceHandle
{
	T_RTTI_CLASS;

public:
	explicit ExplicitResourceHandle(Object* object);
};

	}
}

