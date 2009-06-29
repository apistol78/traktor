#ifndef traktor_resource_IResourceHandle_H
#define traktor_resource_IResourceHandle_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Resource handle interface.
 * \ingroup Resource
 */
class T_DLLCLASS IResourceHandle : public Object
{
	T_RTTI_CLASS(IResourceHandle)

public:
	virtual void replace(Object* object) = 0;

	virtual Object* get() = 0;

	virtual void flush() = 0;
};

	}
}

#endif	// traktor_resource_IResourceHandle_H
