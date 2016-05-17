#ifndef traktor_theater_TheaterEditorProfile_H
#define traktor_theater_TheaterEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

/*! \brief
 */
class T_DLLCLASS TheaterEditorProfile : public scene::ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const T_OVERRIDE T_FINAL;

	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const T_OVERRIDE T_FINAL;

	virtual void createEditorPlugins(
		scene::SceneEditorContext* context,
		RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
	) const T_OVERRIDE T_FINAL;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const T_OVERRIDE T_FINAL;

	virtual void createControllerEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual void createEntityEditorFactories(
		scene::SceneEditorContext* context,
		RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
	) const T_OVERRIDE T_FINAL;

	virtual Ref< world::EntityData > createEntityData(
		scene::SceneEditorContext* context,
		db::Instance* instance
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_theater_TheaterEditorProfile_H
