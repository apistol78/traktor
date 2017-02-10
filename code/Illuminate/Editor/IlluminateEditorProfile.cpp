#include "Illuminate/Editor/IlluminateEditorProfile.h"
#include "Illuminate/Editor/IlluminateEntityFactory.h"
#include "Illuminate/Editor/IlluminateEntityEditorFactory.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminateEditorProfile", 0, IlluminateEditorProfile, scene::ISceneEditorProfile)

void IlluminateEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void IlluminateEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void IlluminateEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void IlluminateEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void IlluminateEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new IlluminateEntityFactory());
}

void IlluminateEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void IlluminateEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void IlluminateEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new IlluminateEntityEditorFactory());
}

void IlluminateEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > IlluminateEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return 0;
}

	}
}
