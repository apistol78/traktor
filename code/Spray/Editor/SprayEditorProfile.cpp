#include "Spray/Editor/SprayEditorProfile.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectEntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SprayEditorProfile", 0, SprayEditorProfile, scene::ISceneEditorProfile)

void SprayEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void SprayEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void SprayEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void SprayEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new spray::EffectFactory(context->getResourceDatabase(), 0));
}

void SprayEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new spray::EffectEntityFactory(context->getResourceManager(), 0, 0));
}

void SprayEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new spray::EffectEntityRenderer(context->getRenderSystem(), 50.0f, 100.0f));
}

void SprayEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void SprayEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

Ref< world::EntityData > SprayEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
