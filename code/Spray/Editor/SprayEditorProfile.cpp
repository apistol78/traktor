#include "Spray/Editor/SprayEditorProfile.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectEntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Heap/GcNew.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.SprayEditorProfile", SprayEditorProfile, scene::ISceneEditorProfile)

void SprayEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void SprayEditorProfile::createToolBarItems(
	ui::custom::ToolBar* toolBar
) const
{
}

void SprayEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< spray::EffectFactory >(context->getResourceDatabase()));
}

void SprayEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< spray::EffectEntityFactory >(context->getResourceManager()));
}

void SprayEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< spray::EffectEntityRenderer >(context->getRenderSystem(), 1.0f, 1.0f));
}

void SprayEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void SprayEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

	}
}
