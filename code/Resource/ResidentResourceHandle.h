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
	ResidentResourceHandle(const TypeInfo& type, bool persistent);

	virtual void replace(Object* object) T_OVERRIDE T_FINAL;

	virtual Object* get() const T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	const TypeInfo& getProductType() const { return m_resourceType; }

	bool isPersistent() const { return m_persistent; }

private:
	const TypeInfo& m_resourceType;
	bool m_persistent;
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResidentResourceHandle_H
