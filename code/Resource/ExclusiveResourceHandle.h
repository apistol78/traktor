#ifndef traktor_resource_ExclusiveResourceHandle_H
#define traktor_resource_ExclusiveResourceHandle_H

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

/*! \brief Exclusive resource handle.
 * \ingroup Resource
 *
 * Exclusive resource handles automatically
 * release contained object when last
 * external reference is released.
 */
class T_DLLCLASS ExclusiveResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	ExclusiveResourceHandle(const TypeInfo& type);

	virtual void release(void* owner) const T_OVERRIDE T_FINAL;

	virtual void replace(Object* object) T_OVERRIDE T_FINAL;

	virtual Object* get() const T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	const TypeInfo& getProductType() const { return m_resourceType; }

private:
	const TypeInfo& m_resourceType;
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ExclusiveResourceHandle_H
