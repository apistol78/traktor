#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/SpatialEntityData.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Heap/New.h"
#include "Ui/Command.h"

// Resource factories
#include "Render/TextureFactory.h"
#include "Render/ShaderFactory.h"
#include "World/PostProcess/PostProcessFactory.h"
#include "Mesh/Static/StaticMeshFactory.h"
#include "Mesh/Skinned/SkinnedMeshFactory.h"
#include "Mesh/Indoor/IndoorMeshFactory.h"
#include "Mesh/Instance/InstanceMeshFactory.h"
#include "Mesh/Blend/BlendMeshFactory.h"

// Entity factories
#include "World/Entity/LightEntityFactory.h"
#include "World/Entity/ExternalEntityFactory.h"
#include "World/Entity/GroupEntityFactory.h"
#include "Weather/WeatherEntityFactory.h"
#include "Mesh/MeshEntityFactory.h"

// Entity renderers
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "Weather/WeatherEntityRenderer.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.scene.DefaultEditorProfile", DefaultEditorProfile, ISceneEditorProfile)

TypeSet DefaultEditorProfile::getEntityDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< world::SpatialEntityData >());
	return typeSet;
}

void DefaultEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void DefaultEditorProfile::createToolBarItems(
	ui::custom::ToolBar* toolBar
) const
{
}

void DefaultEditorProfile::createResourceFactories(
	SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< render::TextureFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< render::ShaderFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< world::PostProcessFactory >(context->getResourceDatabase()));
	outResourceFactories.push_back(gc_new< mesh::StaticMeshFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< mesh::SkinnedMeshFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< mesh::IndoorMeshFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< mesh::InstanceMeshFactory >(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(gc_new< mesh::BlendMeshFactory >(context->getResourceDatabase(), context->getRenderSystem()));
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< world::LightEntityFactory >());
	outEntityFactories.push_back(gc_new< world::ExternalEntityFactory >(context->getSourceDatabase()));
	outEntityFactories.push_back(gc_new< world::GroupEntityFactory >());
	outEntityFactories.push_back(gc_new< weather::WeatherEntityFactory >(context->getResourceManager(), context->getRenderSystem()));
	outEntityFactories.push_back(gc_new< mesh::MeshEntityFactory >(context->getResourceManager()));
}

void DefaultEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< world::LightEntityRenderer >());
	outEntityRenderers.push_back(gc_new< world::GroupEntityRenderer >());
	outEntityRenderers.push_back(gc_new< weather::WeatherEntityRenderer >(primitiveRenderer));
	outEntityRenderers.push_back(gc_new< mesh::MeshEntityRenderer >());
	outEntityRenderers.push_back(gc_new< mesh::InstanceMeshEntityRenderer >());
}

IEntityEditor* DefaultEditorProfile::createEntityEditor(
	SceneEditorContext* context,
	const Type& entityDataType
) const
{
	T_ASSERT (is_type_of< world::SpatialEntityData >(entityDataType));
	return gc_new< DefaultEntityEditor >();
}

	}
}
