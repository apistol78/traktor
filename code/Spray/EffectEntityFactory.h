#pragma once

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

	namespace world
	{

class EntityEventManager;

	}

	namespace spray
	{

class IFeedbackManager;

/*! Effect entity factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	explicit EffectEntityFactory(
		resource::IResourceManager* resourceManager,
		world::EntityEventManager* eventManager,
		sound::ISoundPlayer* soundPlayer,
		IFeedbackManager* feedbackManager
	);

	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const override final;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< world::EntityEventManager > m_eventManager;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	Ref< IFeedbackManager > m_feedbackManager;
};

	}
}

