#ifndef traktor_resource_ResourceHandle_H
#define traktor_resource_ResourceHandle_H

#include "Resource/IResourceHandle.h"

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

/*! \brief Resource handle.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	ResourceHandle(const TypeInfo& type);

	virtual void release(void* owner) const;

	virtual void replace(Object* object);

	virtual Object* get() const;

	virtual void flush();

	const TypeInfo& getResourceType() const;

private:
	const TypeInfo& m_resourceType;
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResourceHandle_H
