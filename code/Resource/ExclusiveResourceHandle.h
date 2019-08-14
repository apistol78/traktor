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

/*! Exclusive resource handle.
 * \ingroup Resource
 *
 * Exclusive resource handles automatically
 * release contained object when last
 * external reference is released.
 */
class T_DLLCLASS ExclusiveResourceHandle : public ResourceHandle
{
	T_RTTI_CLASS;

public:
	ExclusiveResourceHandle(const TypeInfo& type);

	virtual void release(void* owner) const override final;

	/*! Get the type of resource, ie product type. */
	const TypeInfo& getProductType() const { return m_resourceType; }

private:
	const TypeInfo& m_resourceType;
};

	}
}

