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
class IResourceHandle;

/*! \brief Resource manager interface.
 * \ingroup Resource
 */
class T_DLLCLASS IResourceManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	/*! \brief Add resource factory to manager.
	 *
	 * \param factory Resource factory.
	 */
	virtual void addFactory(IResourceFactory* factory) = 0;

	/*! \brief Remove resource factory from manager.
	 *
	 * \param factory Resource factory.
	 */
	virtual void removeFactory(IResourceFactory* factory) = 0;

	/*! \brief Remove all resource factories. */
	virtual void removeAllFactories() = 0;

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param type Type of resource.
	 * \param guid Resource identifier.
	 * \return Resource handle.
	 */
	virtual Ref< IResourceHandle > bind(const TypeInfo& type, const Guid& guid) = 0;

	/*! \brief Update all handles.
	 *
	 * \param guid Resource identifier.
	 * \param force Force update; recreate resource even if resource is cached.
	 */
	virtual void update(const Guid& guid, bool force) = 0;

	/*! \brief Flush resource from cache.
	 *
	 * \param guid Resource identifier.
	 */
	virtual void flush(const Guid& guid) = 0;

	/*! \brief Flush all resources. */
	virtual void flush() = 0;
	
	/*! \brief Dump statistics. */
	virtual void dumpStatistics() = 0;

	template < typename ResourceType >
	Ref< IResourceHandle > bind(const Guid& guid)
	{
		return bind(type_of< ResourceType >(), guid);
	}

	template < typename ResourceType >
	bool bind(Proxy< ResourceType >& proxy)
	{
		Ref< IResourceHandle > handle = bind< ResourceType >(proxy.getGuid());
		if (!handle)
			return false;
		proxy.replace(handle);
		return true;
	}

	template < typename ResourceType >
	void flush(Proxy< ResourceType >& proxy)
	{
		flush(proxy.getGuid());
		proxy.invalidate();
	}
};

	}
}

#endif	// traktor_resource_IResourceManager_H
