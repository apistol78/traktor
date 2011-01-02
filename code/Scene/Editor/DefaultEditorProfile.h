#ifndef traktor_scene_DefaultEditorProfile_H
#define traktor_scene_DefaultEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

namespace traktor
{
	namespace world
	{

class ExternalEntityDataCache;

	}

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

	virtual void createEditorPlugins(
		SceneEditorContext* context,
		RefArray< ISceneEditorPlugin >& outEditorPlugins
	) const;

	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createControllerEditorFactories(
		SceneEditorContext* context,
		RefArray< ISceneControllerEditorFactory >& outControllerEditorFactories
	) const;

	virtual void createEntityEditorFactories(
		SceneEditorContext* context,
		RefArray< IEntityEditorFactory >& outEntityEditorFactories
	) const;

private:
	Ref< world::ExternalEntityDataCache > m_externalCache;
};

	}
}

#endif	// traktor_scene_DefaultEditorProfile_H
