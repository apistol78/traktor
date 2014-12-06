#ifndef traktor_ai_NavMeshEntityFactory_H
#define traktor_ai_NavMeshEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace ai
	{

/*! \brief Navigation mesh entity factory.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	NavMeshEntityFactory(resource::IResourceManager* resourceManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_ai_NavMeshEntityFactory_H
