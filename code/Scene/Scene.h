#ifndef traktor_scene_Scene_H
#define traktor_scene_Scene_H

#include "Core/Heap/Ref.h"
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

	}

	namespace scene
	{

class ISceneController;

class T_DLLCLASS Scene : public Object
{
	T_RTTI_CLASS(Scene)

public:
	Scene(
		ISceneController* controller,
		world::IEntityManager* entityManager,
		world::Entity* rootEntity,
		world::WorldRenderSettings* worldRenderSettings
	);

	ISceneController* getController() const;

	world::IEntityManager* getEntityManager() const;

	world::Entity* getRootEntity() const;

	world::WorldRenderSettings* getWorldRenderSettings() const;

private:
	Ref< world::IEntityManager > m_entityManager;
	Ref< world::Entity > m_rootEntity;
	Ref< ISceneController > m_controller;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
};

	}
}

#endif	// traktor_scene_Scene_H
