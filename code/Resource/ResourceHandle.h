#ifndef traktor_resource_ResourceHandle_H
#define traktor_resource_ResourceHandle_H

#include "Resource/IResourceHandle.h"

namespace traktor
{
	namespace resource
	{

/*! \brief Resource handle.
 * \ingroup Resource
 */
class ResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	ResourceHandle(const TypeInfo& type);

	virtual void replace(Object* object);

	virtual Ref< Object > get();

	virtual void flush();

	const TypeInfo& getResourceType() const;

private:
	const TypeInfo& m_resourceType;
	Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResourceHandle_H
