#ifndef traktor_resource_IResourceFactory_H
#define traktor_resource_IResourceFactory_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;
class Instance;

	}

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
	T_RTTI_CLASS;
	
public:
	/*! \brief Get resource types.
	 *
	 * Return a set of resource types this factory
	 * accepts.
	 *
	 * \return Set of resource types.
	 */
	virtual const TypeInfoSet getResourceTypes() const = 0;

	/*! \brief Get product types from resource.
	 *
	 * Return which product types can be created from a
	 * resource type.
	 *
	 * \param resourceType Type of resource.
	 * \return Set of product types.
	 */
	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const = 0;

	/*! \brief Check if resource is cacheable.
	 *
	 * A non cacheable resource are recreated for each
	 * proxy trying to validate the resource.
	 *
	 * \param productType Type of product.
	 * \return True if resource is cacheable.
	 */
	virtual bool isCacheable(const TypeInfo& productType) const = 0;

	/*! \brief Create resource from guid.
	 *
	 * Create a specified resource from a guid.
	 * It should also fill the outDependencies with
	 * other guid;s if other resources were loaded as
	 * as a result of creating this resource.
	 *
	 * \param resourceManager Resource manager.
	 * \param database Resource database.
	 * \param instance Resource database instance.
	 * \param productType Type of product.
	 * \param original Previously created resource if any; null if first time create.
	 * \return Product.
	 */
	virtual Ref< Object > create(IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const = 0;
};
	
	}
}

#endif	// traktor_resource_IResourceFactory_H
