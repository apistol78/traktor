#include "Database/Instance.h"
#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Command.h"
#include "World/Entity/ExternalEntityData.h"

// Resource factories
#include "Render/ImageProcess/ImageProcessFactory.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "World/EntityEventResourceFactory.h"
#include "World/EntityResourceFactory.h"

// Entity factories
#include "Scene/Editor/LayerEntityFactory.h"
#include "Weather/WeatherFactory.h"
#include "World/Entity/WorldEntityFactory.h"

// Entity renderers
#include "World/Entity/ComponentEntityRenderer.h"
#include "World/Entity/DecalRenderer.h"
#include "World/Entity/LightRenderer.h"
#include "World/Entity/GroupEntityRenderer.h"
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
	outResourceFactories.push_back(new render::ImageProcessFactory());
	outResourceFactories.push_back(new render::SequenceTextureFactory());
	outResourceFactories.push_back(new render::ShaderFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new render::TextureFactory(context->getRenderSystem(), 0));
	outResourceFactories.push_back(new video::VideoFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new weather::CloudMaskFactory());
	outResourceFactories.push_back(new world::EntityResourceFactory());
	//outResourceFactories.push_back(new world::EntityEventResourceFactory());
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new world::WorldEntityFactory(context->getResourceManager(), true));
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
	outEntityRenderers.push_back(new world::ComponentEntityRenderer());
	outEntityRenderers.push_back(new world::DecalRenderer(context->getRenderSystem()));
	outEntityRenderers.push_back(new world::LightRenderer());
	outEntityRenderers.push_back(new world::GroupEntityRenderer());
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
