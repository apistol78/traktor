#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/MaterialMaskFactory.h"
#include "Heightfield/Editor/HeightfieldEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldEditorProfile", 0, HeightfieldEditorProfile, scene::ISceneEditorProfile)

void HeightfieldEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void HeightfieldEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void HeightfieldEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void HeightfieldEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new HeightfieldFactory(context->getResourceDatabase()));
	outResourceFactories.push_back(new MaterialMaskFactory(context->getResourceDatabase()));
}

void HeightfieldEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
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
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void HeightfieldEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

Ref< world::EntityData > HeightfieldEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
