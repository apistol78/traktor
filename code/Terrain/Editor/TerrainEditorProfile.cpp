#include "Core/Serialization/ISerializable.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainFactory.h"
#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEditorProfile", 0, TerrainEditorProfile, scene::ISceneEditorProfile)

void TerrainEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Ocean.RandomizeWaves"));
}

void TerrainEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void TerrainEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new TerrainFactory(context->getResourceDatabase()));
}

void TerrainEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new EntityFactory(
		context->getResourceManager(),
		context->getRenderSystem()
	));
}

void TerrainEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer());
}

void TerrainEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void TerrainEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

Ref< world::EntityData > TerrainEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
