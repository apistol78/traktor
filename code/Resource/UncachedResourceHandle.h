#ifndef traktor_resource_UncachedResourceHandle_H
#define traktor_resource_UncachedResourceHandle_H

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

/*! \brief Uncached resource handle.
 * \ingroup Resource
 *
 * Uncached resource handles automatically
 * release contained object when last
 * external reference is released.
 *
 * As it's not safe to assume resource handle
 * are not used even when wrapped object is null
 * as it can be externally replaced we have
 * to intrude a "in use" member which are
 * explicitly set when external reference
 * is released.
 */
class T_DLLCLASS UncachedResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	UncachedResourceHandle(const TypeInfo& type);

	virtual void release(void* owner) const;

	virtual void replace(Object* object);

	virtual Object* get() const;

	virtual void flush();

	const TypeInfo& getResourceType() const { return m_resourceType; }

	bool inUse() const { return m_inUse; }

private:
	const TypeInfo& m_resourceType;
	mutable Ref< Object > m_object;
	mutable bool m_inUse;
};

	}
}

#endif	// traktor_resource_UncachedResourceHandle_H
