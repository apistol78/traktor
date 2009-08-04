#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Terrain/Editor/OceanEntityEditor.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/OceanEntityData.h"
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainEditorProfile", TerrainEditorProfile, scene::ISceneEditorProfile)

TypeSet TerrainEditorProfile::getEntityDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	return typeSet;
}

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

scene::IEntityEditor* TerrainEditorProfile::createEntityEditor(
	scene::SceneEditorContext* context,
	const Type& entityDataType
) const
{
	if (is_type_of< TerrainEntityData >(entityDataType))
		return gc_new< TerrainEntityEditor >();
	if (is_type_of< OceanEntityEditor >(entityDataType))
		return gc_new< OceanEntityEditor >();
	return 0;
}

	}
}
