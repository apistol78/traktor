#include "Ai/NavMeshEntityFactory.h"
#include "Ai/NavMeshFactory.h"
#include "Ai/Editor/AiEditorProfile.h"
#include "Ai/Editor/NavMeshEntityEditorFactory.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.AiEditorProfile", 0, AiEditorProfile, scene::ISceneEditorProfile)

void AiEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void AiEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Ai.NavMesh");
}

void AiEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void AiEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new NavMeshFactory(context->getResourceDatabase(), true));
}

void AiEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new NavMeshEntityFactory(context->getResourceManager(), true));
}

void AiEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void AiEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void AiEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new NavMeshEntityEditorFactory());
}

Ref< world::EntityData > AiEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}