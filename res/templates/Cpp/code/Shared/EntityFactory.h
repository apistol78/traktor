#ifndef EntityFactory_H
#define EntityFactory_H

#include <Render/IRenderSystem.h>
#include <Resource/IResourceManager.h>
#include <World/IEntityFactory.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_SHARED_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS EntityFactory : public traktor::world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	EntityFactory(traktor::resource::IResourceManager* resourceManager, traktor::render::IRenderSystem* renderSystem);

	virtual const traktor::TypeInfoSet getEntityTypes() const T_OVERRIDE T_FINAL;

	virtual const traktor::TypeInfoSet getEntityEventTypes() const T_OVERRIDE T_FINAL;

	virtual const traktor::TypeInfoSet getEntityComponentTypes() const T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::world::Entity > createEntity(const traktor::world::IEntityBuilder* builder, const traktor::world::EntityData& entityData) const T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::world::IEntityEvent > createEntityEvent(const traktor::world::IEntityBuilder* builder, const traktor::world::IEntityEventData& entityEventData) const T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::world::IEntityComponent > createEntityComponent(const traktor::world::IEntityBuilder* builder, traktor::world::Entity* owner, const traktor::world::IEntityComponentData& entityComponentData) const T_OVERRIDE T_FINAL;

private:
	traktor::Ref< traktor::resource::IResourceManager > m_resourceManager;
	traktor::Ref< traktor::render::IRenderSystem > m_renderSystem;
};

#endif	// EntityFactory_H
