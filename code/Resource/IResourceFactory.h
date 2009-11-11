#ifndef traktor_resource_IResourceFactory_H
#define traktor_resource_IResourceFactory_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"

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

class IResourceManager;

/*! \brief Resource factory interface.
 * \ingroup Resource
 *
 * A resource factory is responsible of creating resources from
 * a given guid.
 * Each factory can support multiple resource types if necessary.
 * It must also provide a list of dependencies to other guid;s
 * as the resource manager needs to know which resources to flush
 * in case of a resource has been modified.
 */
class T_DLLCLASS IResourceFactory : public Object
{
	T_RTTI_CLASS(IResourceFactory)
	
public:
	/*! \brief Get resource types.
	 *
	 * Return a set of resource types this factory
	 * supports, i.e. is capable of creating.
	 */
	virtual const TypeSet getResourceTypes() const = 0;

	/*! \brief Check if resource is cacheable.
	 *
	 * A non cacheable resource are recreated for each
	 * proxy trying to validate the resource.
	 *
	 * \return True if resource is cacheable.
	 */
	virtual bool isCacheable() const = 0;

	/*! \brief Create resource from guid.
	 *
	 * Create a specified resource from a guid.
	 * It should also fill the outDependencies with
	 * other guid;s if other resources were loaded as
	 * as a result of creating this resource.
	 *
	 * \param resourceManager Resource manager.
	 * \param resourceType Type of resource.
	 * \param guid Guid of resource.
	 * \param outCacheable If resource is cacheable (default set to true).
	 * \return Resource instance.
	 */
	virtual Ref< Object > create(IResourceManager* resourceManager, const Type& resourceType, const Guid& guid) = 0;
};
	
	}
}

#endif	// traktor_resource_IResourceFactory_H
