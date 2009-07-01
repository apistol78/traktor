#ifndef traktor_scene_DefaultEditorProfile_H
#define traktor_scene_DefaultEditorProfile_H

#include "Scene/Editor/SceneEditorProfile.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Default scene editor profile. */
class DefaultEditorProfile : public SceneEditorProfile
{
	T_RTTI_CLASS(DefaultEditorProfile)

public:
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
		render::RenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createEntityEditors(
		SceneEditorContext* context,
		RefArray< EntityEditor >& outEntityEditors
	) const;
};

	}
}

#endif	// traktor_scene_DefaultEditorProfile_H
