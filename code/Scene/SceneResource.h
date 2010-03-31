#ifndef traktor_scene_SceneResource_H
#define traktor_scene_SceneResource_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Proxy.h"
#include "World/WorldRenderSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
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

	namespace render
	{

class IRenderSystem;

	}

	namespace world
	{

class IEntityBuilder;
class IEntityManager;
class EntityData;
class WorldRenderSettings;
class PostProcessSettings;

	}

	namespace scene
	{

class ISceneControllerData;
class Scene;

class T_DLLCLASS SceneResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SceneResource();

	Ref< Scene > createScene(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		world::IEntityBuilder* entityBuilder,
		world::IEntityManager* entityManager,
		world::WorldRenderSettings::ShadowQuality shadowQuality
	) const;

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	void setPostProcessSettings(const resource::Proxy< world::PostProcessSettings >& postProcess);

	const resource::Proxy< world::PostProcessSettings >& getPostProcessSettings() const;

	void setEntityData(world::EntityData* entityData);

	Ref< world::EntityData > getEntityData() const;

	void setControllerData(ISceneControllerData* controllerData);

	Ref< ISceneControllerData > getControllerData() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	mutable resource::Proxy< world::PostProcessSettings > m_postProcessSettings;
	Ref< world::EntityData > m_entityData;
	Ref< ISceneControllerData > m_controllerData;
};

	}
}

#endif	// traktor_scene_SceneResource_H
