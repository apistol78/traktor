/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	) const T_OVERRIDE T_FINAL;

	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const T_OVERRIDE T_FINAL;

	virtual void createEditorPlugins(
		SceneEditorContext* context,
		RefArray< ISceneEditorPlugin >& outEditorPlugins
	) const T_OVERRIDE T_FINAL;

	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const T_OVERRIDE T_FINAL;

	virtual void createControllerEditorFactories(
		SceneEditorContext* context,
		RefArray< const ISceneControllerEditorFactory >& outControllerEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityEditorFactories(
		SceneEditorContext* context,
		RefArray< const IEntityEditorFactory >& outEntityEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createComponentEditorFactories(
		SceneEditorContext* context,
		RefArray< const IComponentEditorFactory >& outComponentEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual Ref< world::EntityData > createEntityData(
		SceneEditorContext* context,
		db::Instance* instance
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_scene_DefaultEditorProfile_H
