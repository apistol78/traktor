#ifndef traktor_resource_ResourceHandle_H
#define traktor_resource_ResourceHandle_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(ResourceHandle)

public:
	ResourceHandle(const Type& type);

	virtual void replace(Object* object);

	virtual Ref< Object > get();

	virtual void flush();

	const Type& getResourceType() const;

private:
	const Type& m_resourceType;
	Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResourceHandle_H
