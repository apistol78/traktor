#ifndef traktor_spray_EffectEntityFactory_H
#define traktor_spray_EffectEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
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

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spray
	{

/*! \brief Effect entity factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	EffectEntityFactory(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;

private:
	resource::IResourceManager* m_resourceManager;
	sound::ISoundPlayer* m_soundPlayer;
};

	}
}

#endif	// traktor_spray_EffectEntityFactory_H
