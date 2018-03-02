#ifndef EditorProfile_H
#define EditorProfile_H

#include <Scene/Editor/ISceneEditorProfile.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS EditorProfile : public traktor::scene::ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< traktor::ui::Command >& outCommands) const T_OVERRIDE T_FINAL;

	virtual void getGuideDrawIds(std::set< std::wstring >& outIds) const T_OVERRIDE T_FINAL;

	virtual void createEditorPlugins(
		traktor::scene::SceneEditorContext* context,
		traktor::RefArray< traktor::scene::ISceneEditorPlugin >& outEditorPlugins
	) const T_OVERRIDE T_FINAL;

	virtual void createResourceFactories(
		traktor::scene::SceneEditorContext* context,
		traktor::RefArray< const traktor::resource::IResourceFactory >& outResourceFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityFactories(
		traktor::scene::SceneEditorContext* context,
		traktor::RefArray< const traktor::world::IEntityFactory >& outEntityFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityRenderers(
		traktor::scene::SceneEditorContext* context,
		traktor::render::IRenderView* renderView,
		traktor::render::PrimitiveRenderer* primitiveRenderer,
		traktor::RefArray< traktor::world::IEntityRenderer >& outEntityRenderers
	) const T_OVERRIDE T_FINAL;

	virtual void createControllerEditorFactories(
		traktor::scene::SceneEditorContext* context,
		traktor::RefArray< const traktor::scene::ISceneControllerEditorFactory >& outControllerEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityEditorFactories(
		traktor::scene::SceneEditorContext* context,
		traktor::RefArray< const traktor::scene::IEntityEditorFactory >& outEntityEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual traktor::Ref< traktor::world::EntityData > createEntityData(
		traktor::scene::SceneEditorContext* context,
		traktor::db::Instance* instance
	) const T_OVERRIDE T_FINAL;
};

#endif	// EditorProfile_H
