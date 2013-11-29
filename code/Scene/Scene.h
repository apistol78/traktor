#ifndef traktor_scene_Scene_H
#define traktor_scene_Scene_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/PostProcess/PostProcessSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

	}

	namespace world
	{

class Entity;
class IEntitySchema;
class IWorldRenderer;
struct UpdateParams;
class WorldRenderSettings;
class WorldRenderView;

	}

	namespace scene
	{

class ISceneController;

/*! \brief Scene
 * \ingroup Scene
 *
 * A scene holds information about entities,
 * world renderer configuration and post processing.
 */
class T_DLLCLASS Scene : public Object
{
	T_RTTI_CLASS;

public:
	Scene(
		ISceneController* controller,
		world::IEntitySchema* entitySchema,
		world::Entity* rootEntity,
		world::WorldRenderSettings* worldRenderSettings,
		const resource::Proxy< world::PostProcessSettings >& postProcessSettings,
		const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& postProcessParams
	);

	virtual ~Scene();

	void destroy();

	void update(const world::UpdateParams& update, bool updateController, bool updateEntity);

	world::IEntitySchema* getEntitySchema() const;

	world::Entity* getRootEntity() const;

	ISceneController* getController() const;

	world::WorldRenderSettings* getWorldRenderSettings() const;

	const resource::Proxy< world::PostProcessSettings >& getPostProcessSettings() const;

	const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& getPostProcessParams() const;

private:
	Ref< world::IEntitySchema > m_entitySchema;
	Ref< world::Entity > m_rootEntity;
	Ref< ISceneController > m_controller;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	resource::Proxy< world::PostProcessSettings > m_postProcessSettings;
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > m_postProcessParams;
};

	}
}

#endif	// traktor_scene_Scene_H
