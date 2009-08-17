#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Terrain/Editor/TerrainEntityEditorFactory.h"
#include "Terrain/HeightfieldFactory.h"
#include "Terrain/MaterialMaskFactory.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Heap/GcNew.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainEditorProfile", TerrainEditorProfile, scene::ISceneEditorProfile)

void TerrainEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Terrain.ToggleFollowGround"));
	outCommands.push_back(ui::Command(L"Terrain.FlushSurfaceCache"));
	outCommands.push_back(ui::Command(L"Ocean.RandomizeWaves"));
}

void TerrainEditorProfile::createToolBarItems(
	ui::custom::ToolBar* toolBar
) const
{
}

void TerrainEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< HeightfieldFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< MaterialMaskFactory >(context->getResourceDatabase()));
}

void TerrainEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< EntityFactory >(
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
	outEntityRenderers.push_back(gc_new< EntityRenderer >());
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
	outEntityEditorFactories.push_back(gc_new< TerrainEntityEditorFactory >());
}

	}
}
