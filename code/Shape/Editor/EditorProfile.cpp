#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/EntityFactory.h"
#include "Shape/EntityRenderer.h"
#include "Shape/Editor/EditorProfile.h"
#include "Shape/Editor/Prefab/PrefabEntityFactory.h"
#include "Shape/Editor/Prefab/PrefabEntityEditorFactory.h"
#include "Shape/Editor/Solid/SolidEditorPlugin.h"
#include "Shape/Editor/Solid/SolidEntityEditorFactory.h"
#include "Shape/Editor/Solid/SolidEntityFactory.h"
#include "Shape/Editor/Solid/SolidEntityRenderer.h"
#include "Shape/Editor/Spline/ControlPointEntityEditorFactory.h"
#include "Shape/Editor/Spline/SplineEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.EditorProfile", 0, EditorProfile, scene::ISceneEditorProfile)

void EditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Shape.Editor.EditPrimitive"));
}

void EditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Shape.Solids");
}

void EditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
	outEditorPlugins.push_back(new SolidEditorPlugin(context));
}

void EditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void EditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new PrefabEntityFactory());
	outEntityFactories.push_back(new SolidEntityFactory(context->getResourceManager(), context->getRenderSystem()));
	outEntityFactories.push_back(new EntityFactory(context->getResourceManager(), context->getRenderSystem()));
}

void EditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer());
	outEntityRenderers.push_back(new SolidEntityRenderer());
}

void EditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void EditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new ControlPointEntityEditorFactory());
	outEntityEditorFactories.push_back(new PrefabEntityEditorFactory());
	outEntityEditorFactories.push_back(new SolidEntityEditorFactory());
	outEntityEditorFactories.push_back(new SplineEntityEditorFactory());
}

void EditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > EditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

void EditorProfile::getDebugTargets(
	scene::SceneEditorContext* context,
	std::vector< render::DebugTarget >& outDebugTargets
) const
{
}

	}
}
