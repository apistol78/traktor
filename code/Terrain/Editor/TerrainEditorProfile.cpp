#include "Core/Serialization/ISerializable.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Terrain/TerrainFactory.h"
#include "Terrain/Editor/OceanEntityEditorFactory.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Terrain/Editor/TerrainEntityEditorFactory.h"
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
	outCommands.push_back(ui::Command(L"Terrain.Editor.EditTerrain"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.AverageBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.ColorBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.CutBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.ElevateBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.EmissiveBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.FlattenBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.MaterialBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.NoiseBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SmoothBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SmoothFallOff"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SharpFallOff"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SymmetryX"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SymmetryZ"));
}

void TerrainEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Terrain.Heightfield");
}

void TerrainEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
	outEditorPlugins.push_back(new TerrainEditorPlugin(context));
}

void TerrainEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new TerrainFactory(context->getResourceDatabase()));
}

void TerrainEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
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
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void TerrainEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new OceanEntityEditorFactory());
	outEntityEditorFactories.push_back(new TerrainEntityEditorFactory());
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
