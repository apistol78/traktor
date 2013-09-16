#ifndef traktor_scene_DefaultEditorProfile_H
#define traktor_scene_DefaultEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Default scene editor profile. */
class DefaultEditorProfile : public ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	DefaultEditorProfile();

	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const;

	virtual void createEditorPlugins(
		SceneEditorContext* context,
		RefArray< ISceneEditorPlugin >& outEditorPlugins
	) const;

	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createControllerEditorFactories(
		SceneEditorContext* context,
		RefArray< const ISceneControllerEditorFactory >& outControllerEditorFactories
	) const;

	virtual void createEntityEditorFactories(
		SceneEditorContext* context,
		RefArray< const IEntityEditorFactory >& outEntityEditorFactories
	) const;

	virtual Ref< world::EntityData > createEntityData(
		SceneEditorContext* context,
		db::Instance* instance
	) const;
};

	}
}

#endif	// traktor_scene_DefaultEditorProfile_H
