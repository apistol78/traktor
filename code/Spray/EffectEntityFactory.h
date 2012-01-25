#ifndef traktor_spray_EffectEntityFactory_H
#define traktor_spray_EffectEntityFactory_H

#include "World/Entity/IEntityFactory.h"

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

class SoundSystem;

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
	EffectEntityFactory(resource::IResourceManager* resourceManager, sound::SoundSystem* soundSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< world::Entity > createEntity(world::IEntityBuilder* builder, const world::EntityData& entityData) const;

private:
	resource::IResourceManager* m_resourceManager;
	sound::SoundSystem* m_soundSystem;
};

	}
}

#endif	// traktor_spray_EffectEntityFactory_H
