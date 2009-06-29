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
	virtual void replace(Object* object);

	virtual Object* get();

	virtual void flush();

private:
	Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ResourceHandle_H
