#include "Amalgam/Editor/SceneEditorProfile.h"
#include "Amalgam/Game/Engine/GameEntityFactory.h"
#include "Amalgam/Game/Engine/GameEntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.SceneEditorProfile", 0, SceneEditorProfile, scene::ISceneEditorProfile)

void SceneEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void SceneEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void SceneEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void SceneEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void SceneEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new GameEntityFactory(context->getResourceManager()));
}

void SceneEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new GameEntityRenderer());
}

void SceneEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void SceneEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

Ref< world::EntityData > SceneEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
