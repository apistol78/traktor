#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Terrain/Editor/OceanEntityEditor.h"
#include "Terrain/HeightfieldFactory.h"
#include "Terrain/MaterialMaskFactory.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Heap/New.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainEditorProfile", TerrainEditorProfile, scene::SceneEditorProfile)

void TerrainEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Terrain.ToggleFollowGround"));
	outCommands.push_back(ui::Command(L"Terrain.FlushSurfaceCache"));
	outCommands.push_back(ui::Command(L"Ocean.RandomizeWaves"));
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
	RefArray< world::EntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< EntityFactory >(
		context->getResourceManager(),
		context->getRenderSystem()
	));
}

void TerrainEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::RenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::EntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< EntityRenderer >());
}

void TerrainEditorProfile::createEntityEditors(
	scene::SceneEditorContext* context,
	RefArray< scene::EntityEditor >& outEntityEditors
) const
{
	outEntityEditors.push_back(gc_new< TerrainEntityEditor >());
	outEntityEditors.push_back(gc_new< OceanEntityEditor >());
}

	}
}
