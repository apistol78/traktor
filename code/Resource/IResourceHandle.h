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
	/*! \brief Replace resource object.
	 *
	 * \param object New resource object.
	 */
	virtual void replace(Object* object) = 0;

	/*! \brief Get resource object.
	 *
	 * \return Resource object.
	 */
	virtual Object* get() = 0;

	/*! \brief Flush resource object. */
	virtual void flush() = 0;
};

	}
}

#endif	// traktor_resource_IResourceHandle_H
