#ifndef traktor_terrain_TerrainFactory_H
#define traktor_terrain_TerrainFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

/*! \brief Terrain resource factory.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_terrain_TerrainFactory_H
