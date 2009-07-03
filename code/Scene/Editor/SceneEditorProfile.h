#ifndef traktor_scene_SceneEditorProfile_H
#define traktor_scene_SceneEditorProfile_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Command;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace render
	{

class RenderView;
class PrimitiveRenderer;

	}

	namespace resource
	{

class IResourceFactory;

	}

	namespace world
	{

class WorldRenderer;
class IEntityFactory;
class IEntityRenderer;

	}

	namespace scene
	{

class SceneEditorContext;
class IEntityEditor;

/*! \brief Scene editor profile.
 * \ingroup Scene
 *
 * Profiles are instantiated automatically by the
 * scene editor and are used to create necessary
 * factories.
 */
class T_DLLCLASS SceneEditorProfile : public Object
{
	T_RTTI_CLASS(SceneEditorProfile)

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createToolBarItems(
		ui::custom::ToolBar* toolBar
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
		render::RenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual void createEntityEditors(
		SceneEditorContext* context,
		RefArray< IEntityEditor >& outEntityEditors
	) const;

	virtual void setupWorldRenderer(
		SceneEditorContext* context,
		world::WorldRenderer* worldRenderer
	) const;
};

	}
}

#endif	// traktor_scene_SceneEditorProfile_H
