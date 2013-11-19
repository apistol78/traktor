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
class ResourceBundle;

/*! \brief Resource manager statistics.
 * \ingroup Resource
 */
struct ResourceManagerStatistics
{
	uint32_t residentCount;		//!< Number of resident resources.
	uint32_t exclusiveCount;	//!< Number of exclusive (non-shareable) resources.

	ResourceManagerStatistics()
	:	residentCount(0)
	,	exclusiveCount(0)
	{
	}
};

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
	virtual void addFactory(const IResourceFactory* factory) = 0;

	/*! \brief Remove resource factory from manager.
	 *
	 * \param factory Resource factory.
	 */
	virtual void removeFactory(const IResourceFactory* factory) = 0;

	/*! \brief Remove all resource factories. */
	virtual void removeAllFactories() = 0;

	/*! \brief Load all resources in bundle.
	 *
	 * \param bundle Resource bundle.
	 * \return True if all resources loaded successfully.
	 */
	virtual bool load(const ResourceBundle* bundle) = 0;

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

	/*! \brief Reload all resources of given type.
	 *
	 * \param type Type of resource.
	 */
	virtual void reload(const TypeInfo& type) = 0;

	/*! \brief Unload externally unused, resident, resources.
	 *
	 * Call this when unused resources which are resident can
	 * be unloaded.
	 */
	virtual void unloadUnusedResident() = 0;

	/*! \brief Get statistics. */
	virtual void getStatistics(ResourceManagerStatistics& outStatistics) const = 0;

	/*! \brief Bind handle to resource identifier.
	 *
	 * \param id Resource identifier.
	 * \return Resource proxy.
	 */
	template < 
		typename ResourceType,
		typename ProductType
	>
	bool bind(const Id< ResourceType >& id, Proxy< ProductType >& outProxy)
	{
		Ref< IResourceHandle > handle = bind(type_of< ProductType >(), id);
		if (!handle)
			return false;

		outProxy = Proxy< ProductType >(handle);
		return bool(handle->get() != 0);
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
		return bool(handle->get() != 0);
	}
};

	}
}

#endif	// traktor_resource_IResourceManager_H
