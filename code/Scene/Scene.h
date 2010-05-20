#ifndef traktor_scene_Scene_H
#define traktor_scene_Scene_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class IEntityManager;
class PostProcessSettings;
class WorldRenderer;
class WorldRenderSettings;
class WorldRenderView;

	}

	namespace scene
	{

class ISceneController;

class T_DLLCLASS Scene : public Object
{
	T_RTTI_CLASS;

public:
	Scene(
		ISceneController* controller,
		world::IEntityManager* entityManager,
		world::Entity* rootEntity,
		world::WorldRenderSettings* worldRenderSettings,
		world::PostProcessSettings* postProcessSettings
	);

	virtual ~Scene();

	void destroy();

	void update(float time, float deltaTime);

	void build(world::WorldRenderer* worldRenderer, world::WorldRenderView& worldRenderView, int frame);

	Ref< world::IEntityManager > getEntityManager() const;

	Ref< world::Entity > getRootEntity() const;

	Ref< ISceneController > getController() const;

	Ref< world::WorldRenderSettings > getWorldRenderSettings() const;

	Ref< world::PostProcessSettings > getPostProcessSettings() const;

private:
	Ref< world::IEntityManager > m_entityManager;
	Ref< world::Entity > m_rootEntity;
	Ref< ISceneController > m_controller;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::PostProcessSettings > m_postProcessSettings;
};

	}
}

#endif	// traktor_scene_Scene_H
