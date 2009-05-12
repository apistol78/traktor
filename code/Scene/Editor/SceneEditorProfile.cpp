#include "Scene/Editor/SceneEditorProfile.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorProfile", SceneEditorProfile, Object)

void SceneEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void SceneEditorProfile::createToolBarItems(
	ui::custom::ToolBar* toolBar
) const
{
}

void SceneEditorProfile::createResourceFactories(
	SceneEditorContext* context,
	RefArray< resource::ResourceFactory >& outResourceFactories
) const
{
}

void SceneEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< world::EntityFactory >& outEntityFactories
) const
{
}

void SceneEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::RenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::EntityRenderer >& outEntityRenderers
) const
{
}

void SceneEditorProfile::createEntityEditors(
	SceneEditorContext* context,
	RefArray< EntityEditor >& outEntityEditors
) const
{
}

void SceneEditorProfile::setupWorldRenderer(
	SceneEditorContext* context,
	world::WorldRenderer* worldRenderer
) const
{
}

	}
}
