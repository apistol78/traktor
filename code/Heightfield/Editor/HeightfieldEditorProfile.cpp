#include "Core/Serialization/ISerializable.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/MaterialMaskFactory.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Heightfield/Editor/HeightfieldEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldEditorProfile", 0, HeightfieldEditorProfile, scene::ISceneEditorProfile)

void HeightfieldEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Heightfield.Save"));
}

void HeightfieldEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
	outEditorPlugins.push_back(new HeightfieldEditorPlugin(context));
}

void HeightfieldEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new HeightfieldFactory(context->getResourceDatabase()));
	outResourceFactories.push_back(new MaterialMaskFactory(context->getResourceDatabase()));
}

void HeightfieldEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
}

void HeightfieldEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void HeightfieldEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void HeightfieldEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

	}
}
