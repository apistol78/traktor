#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/Editor/EditorProfile.h"
#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/EntityRenderer.h"
#include "Shape/Editor/Prefab/PrefabEntityFactory.h"
#include "Shape/Editor/Prefab/PrefabEntityEditorFactory.h"
#include "Shape/Editor/Solid/SolidEditorPlugin.h"
#include "Shape/Editor/Solid/SolidEntityEditorFactory.h"
#include "Shape/Editor/Spline/ControlPointComponentEditorFactory.h"
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
	outCommands.push_back(ui::Command(L"Shape.Editor.BrowseMaterial"));
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
	std::wstring assetPath = context->getEditor()->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	std::wstring modelCachePath = context->getEditor()->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCachePath", L"");

	outEntityFactories.push_back(new PrefabEntityFactory());
	outEntityFactories.push_back(new EntityFactory(
		context->getSourceDatabase(),
		context->getResourceManager(),
		context->getRenderSystem(),
		assetPath,
		modelCachePath
	));
}

void EditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer());
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
	outEntityEditorFactories.push_back(new PrefabEntityEditorFactory());
	outEntityEditorFactories.push_back(new SolidEntityEditorFactory());
	outEntityEditorFactories.push_back(new SplineEntityEditorFactory());
}

void EditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new ControlPointComponentEditorFactory());
}

Ref< world::EntityData > EditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

	}
}
