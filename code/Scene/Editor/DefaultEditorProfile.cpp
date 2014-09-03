#include "Database/Instance.h"
#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Command.h"
#include "World/Entity/ExternalEntityData.h"

// Resource factories
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityResourceFactory.h"
#include "World/PostProcess/PostProcessFactory.h"

// Entity factories
#include "Scene/Editor/LayerEntityFactory.h"
#include "Weather/WeatherEntityFactory.h"
#include "World/Entity/WorldEntityFactory.h"

// Entity renderers
#include "World/Entity/DecalEntityRenderer.h"
#include "World/Entity/LightEntityRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/SwitchEntityRenderer.h"
#include "World/Entity/TransientEntityRenderer.h"
#include "Weather/WeatherEntityRenderer.h"

// Entity editor factories
#include "Scene/Editor/DefaultEntityEditorFactory.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.DefaultEditorProfile", 0, DefaultEditorProfile, ISceneEditorProfile)

DefaultEditorProfile::DefaultEditorProfile()
{
}

void DefaultEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void DefaultEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Entity.Light");
	outIds.insert(L"Entity.BoundingBox");
	outIds.insert(L"Entity.Volumes");
}

void DefaultEditorProfile::createEditorPlugins(
	SceneEditorContext* context,
	RefArray< ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void DefaultEditorProfile::createResourceFactories(
	SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new render::ShaderFactory(context->getResourceDatabase(), context->getRenderSystem()));
	outResourceFactories.push_back(new render::TextureFactory(context->getResourceDatabase(), context->getRenderSystem(), 0));
	outResourceFactories.push_back(new weather::CloudMaskFactory(context->getResourceDatabase()));
	outResourceFactories.push_back(new world::EntityResourceFactory(context->getResourceDatabase()));
	//outResourceFactories.push_back(new world::EntityEventResourceFactory(context->getResourceDatabase()));
	outResourceFactories.push_back(new world::PostProcessFactory(context->getResourceDatabase()));
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new world::WorldEntityFactory(context->getResourceManager(), true));
	outEntityFactories.push_back(new weather::WeatherEntityFactory(context->getResourceManager(), context->getRenderSystem()));
	outEntityFactories.push_back(new LayerEntityFactory());
}

void DefaultEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new world::DecalEntityRenderer(context->getRenderSystem()));
	outEntityRenderers.push_back(new world::LightEntityRenderer());
	outEntityRenderers.push_back(new world::GroupEntityRenderer());
	outEntityRenderers.push_back(new world::SwitchEntityRenderer());
	outEntityRenderers.push_back(new world::TransientEntityRenderer());
	outEntityRenderers.push_back(new weather::WeatherEntityRenderer(primitiveRenderer));
}

void DefaultEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void DefaultEditorProfile::createEntityEditorFactories(
	SceneEditorContext* context,
	RefArray< const IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new DefaultEntityEditorFactory());
}

Ref< world::EntityData > DefaultEditorProfile::createEntityData(
	SceneEditorContext* context,
	db::Instance* instance
) const
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	if (!primaryType)
		return 0;

	if (!is_type_of< world::EntityData >(*primaryType))
		return 0;

	// Create external reference to entity data.
	Ref< world::EntityData > entityData = new world::ExternalEntityData(resource::Id< world::EntityData >(instance->getGuid()));
	entityData->setName(instance->getName());

	return entityData;
}

	}
}
