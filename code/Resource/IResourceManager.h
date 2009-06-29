#ifndef traktor_resource_IResourceManager_H
#define traktor_resource_IResourceManager_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Resource/Proxy.h"

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

class IResourceFactory;
class IResourceCache;
class IResourceHandle;

/*! \brief Resource manager interface.
 * \ingroup Resource
 */
class T_DLLCLASS IResourceManager : public Object
{
	T_RTTI_CLASS(IResourceManager)

public:
	virtual void addFactory(IResourceFactory* factory) = 0;

	virtual void removeFactory(IResourceFactory* factory) = 0;

	virtual void removeAllFactories() = 0;

	virtual void setCache(IResourceCache* cache) = 0;

	virtual IResourceCache* getCache() const = 0;

	virtual IResourceHandle* bind(const Type& type, const Guid& guid) = 0;

	template < typename ResourceType >
	IResourceHandle* bind(const Guid& guid)
	{
		return bind(type_of< ResourceType >(), guid);
	}

	template < typename ResourceType >
	bool bind(Proxy< ResourceType >& proxy)
	{
		Ref< IResourceHandle > handle = bind< ResourceType >(proxy.getGuid());
		T_ASSERT (handle);

		proxy.replace(handle);
		return true;
	}
};

	}
}

#endif	// traktor_resource_IResourceManager_H
