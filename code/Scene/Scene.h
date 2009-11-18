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

class IEntityManager;
class Entity;
class WorldRenderSettings;
class PostProcessSettings;

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
