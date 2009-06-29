#ifndef traktor_spray_SprayEditorProfile_H
#define traktor_spray_SprayEditorProfile_H

#include "Scene/Editor/SceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS SprayEditorProfile : public scene::SceneEditorProfile
{
	T_RTTI_CLASS(SprayEditorProfile)

public:
	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< world::EntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::RenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::EntityRenderer >& outEntityRenderers
	) const;
};

	}
}

#endif	// traktor_spray_SprayEditorProfile_H
