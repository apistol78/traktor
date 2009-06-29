#ifndef traktor_terrain_TerrainEditorProfile_H
#define traktor_terrain_TerrainEditorProfile_H

#include "Scene/Editor/SceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS TerrainEditorProfile : public scene::SceneEditorProfile
{
	T_RTTI_CLASS(TerrainEditorProfile)

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

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

	virtual void createEntityEditors(
		scene::SceneEditorContext* context,
		RefArray< scene::EntityEditor >& outEntityEditors
	) const;
};

	}
}

#endif	// traktor_terrain_TerrainEditorProfile_H
