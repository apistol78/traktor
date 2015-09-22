#ifndef traktor_amalgam_GameEntityFactory_H
#define traktor_amalgam_GameEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
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

	namespace world
	{

class IEntityEventManager;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS GameEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	GameEntityFactory(resource::IResourceManager* resourceManager, world::IEntityEventManager* eventManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(
		const world::IEntityBuilder* builder,
		const world::EntityData& entityData
	) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(
		const world::IEntityBuilder* builder,
		const world::IEntityEventData& entityEventData
	) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< world::IEntityEventManager > m_eventManager;
};

	}
}

#endif	// traktor_amalgam_GameEntityFactory_H
