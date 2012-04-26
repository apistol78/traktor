#ifndef traktor_resource_ResidentResourceHandle_H
#define traktor_resource_ResidentResourceHandle_H

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

/*! \brief Cached resource handle.
 * \ingroup Resource
 *
 * Cached resource persist in the resource manager
 * thus are never reloaded unless explicitly flushed.
 */
class T_DLLCLASS ResidentResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	ResidentResourceHandle(const TypeInfo& type);

	virtual void replace(Object* object);

	virtual Object* get() const;

	virtual void flush();

	const TypeInfo& getResourceType() const { return m_resourceType; }

private:
	const TypeInfo& m_resourceType;
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResidentResourceHandle_H
