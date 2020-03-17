#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity/ExternalEntityData.h"

// Resource factories
#include "Render/Image2/ImageGraphFactory.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Script/ScriptFactory.h"
#include "Sound/SoundFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "World/WorldResourceFactory.h"

// Entity factories
#include "Scene/Editor/LayerEntityFactory.h"
#include "Weather/WeatherFactory.h"
#include "World/Entity/WorldEntityFactory.h"

// Entity renderers
#include "World/EntityRenderer.h"
#include "World/Entity/DecalRenderer.h"
#include "World/Entity/LightRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
#include "World/Entity/ProbeRenderer.h"
#include "Weather/WeatherRenderer.h"

// Entity editor factories
#include "Scene/Editor/DefaultComponentEditorFactory.h"
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
	outResourceFactories.push_back(new render::ImageGraphFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new render::SequenceTextureFactory());
	outResourceFactories.push_back(new render::ShaderFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new render::TextureFactory(context->getRenderSystem(), 0));
	outResourceFactories.push_back(new sound::SoundFactory());
	outResourceFactories.push_back(new video::VideoFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new weather::CloudMaskFactory());
	outResourceFactories.push_back(new world::WorldResourceFactory(context->getRenderSystem(), nullptr));
	outResourceFactories.push_back(new script::ScriptFactory(context->getScriptContext()));
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new world::WorldEntityFactory(context->getResourceManager(), context->getRenderSystem(), true));
	outEntityFactories.push_back(new weather::WeatherFactory(context->getResourceManager(), context->getRenderSystem()));
	outEntityFactories.push_back(new LayerEntityFactory());
}

void DefaultEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new world::EntityRenderer());
	outEntityRenderers.push_back(new world::DecalRenderer(context->getRenderSystem()));
	outEntityRenderers.push_back(new world::ProbeRenderer(context->getResourceManager(), context->getRenderSystem(), type_of< world::WorldRendererDeferred >()));
	outEntityRenderers.push_back(new world::LightRenderer());
	outEntityRenderers.push_back(new world::GroupEntityRenderer(world::EmAll));
	outEntityRenderers.push_back(new weather::WeatherRenderer(primitiveRenderer));
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

void DefaultEditorProfile::createComponentEditorFactories(
	SceneEditorContext* context,
	RefArray< const IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new DefaultComponentEditorFactory());
}

Ref< world::EntityData > DefaultEditorProfile::createEntityData(
	SceneEditorContext* context,
	db::Instance* instance
) const
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	if (!primaryType)
		return nullptr;

	if (!is_type_of< world::EntityData >(*primaryType))
		return nullptr;

	// Create external reference to entity data.
	Ref< world::EntityData > entityData = new world::ExternalEntityData(resource::Id< world::EntityData >(instance->getGuid()));
	entityData->setName(instance->getName());
	return entityData;
}

void DefaultEditorProfile::getDebugTargets(
	SceneEditorContext* context,
	std::vector< render::DebugTarget >& outDebugTargets
) const
{
}

	}
}
