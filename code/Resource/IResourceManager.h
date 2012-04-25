#ifndef traktor_resource_IResourceManager_H
#define traktor_resource_IResourceManager_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Resource/Id.h"
#include "Resource/IdProxy.h"
#include "Resource/Proxy.h"

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

	/*! \brief Reload resource.
	 *
	 * \param guid Resource identifier.
	 */
	virtual void reload(const Guid& guid) = 0;

	/*! \brief Flush resource from cache.
	 *
	 * All proxies referencing resource become null pointers
	 * thus this must be called when it's safe and the resource
	 * should no longer be used.
	 *
	 * \param guid Resource identifier.
	 */
	virtual void flush(const Guid& guid) = 0;

	/*! \brief Flush all resources.
	 *
	 * All proxies referencing any resource become null pointers
	 * thus this must be called when it's safe and the resource
	 * should no longer be used.
	 */
	virtual void flush() = 0;
	
	/*! \brief Dump statistics. */
	virtual void dumpStatistics() = 0;

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param id Resource identifier.
	 * \return Resource proxy.
	 */
	template < 
		typename ResourceType
	>
	bool bind(const Id< ResourceType >& id, Proxy< ResourceType >& outProxy)
	{
		Ref< IResourceHandle > handle = bind(type_of< ResourceType >(), id);
		if (!handle)
			return false;

		outProxy = Proxy< ResourceType >(handle);
		return true;
	}

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param proxy Resource identifier proxy.
	 */
	template <
		typename ResourceType
	>
	bool bind(IdProxy< ResourceType >& outProxy)
	{
		Ref< IResourceHandle > handle = bind(type_of< ResourceType >(), outProxy.getId());
		if (!handle)
			return false;

		outProxy.replace(handle);
		return true;
	}
};

	}
}

#endif	// traktor_resource_IResourceManager_H
