#include "Theater/Editor/TheaterEditorProfile.h"
#include "Theater/Editor/TheaterControllerEditorFactory.h"
#include "Ui/Command.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterEditorProfile", TheaterEditorProfile, scene::ISceneEditorProfile)

void TheaterEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Theater.CaptureEntities"));
	outCommands.push_back(ui::Command(L"Theater.DeleteSelectedKey"));
	outCommands.push_back(ui::Command(L"Theater.GotoPreviousKey"));
	outCommands.push_back(ui::Command(L"Theater.GotoNextKey"));
}

void TheaterEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void TheaterEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
}

void TheaterEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
}

void TheaterEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void TheaterEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
	outControllerEditorFactories.push_back(new TheaterControllerEditorFactory());
}

void TheaterEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

	}
}
