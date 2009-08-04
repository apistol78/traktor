#include "Animation/Editor/AnimationEditorProfile.h"
#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntityFactory.h"
#include "Animation/PathEntity/PathEntityRenderer.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.AnimationEditorProfile", AnimationEditorProfile, scene::ISceneEditorProfile)

TypeSet AnimationEditorProfile::getEntityDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

void AnimationEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Animation.Editor.StepForward"));
	outCommands.push_back(ui::Command(L"Animation.Editor.StepBack"));
	outCommands.push_back(ui::Command(L"Animation.Editor.GotoPreviousKey"));
	outCommands.push_back(ui::Command(L"Animation.Editor.GotoNextKey"));
	outCommands.push_back(ui::Command(L"Animation.Editor.InsertKey"));
}

void AnimationEditorProfile::createToolBarItems(
	ui::custom::ToolBar* toolBar
) const
{
}

void AnimationEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< AnimationFactory >(context->getResourceDatabase()));
}

void AnimationEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< AnimatedMeshEntityFactory >(context->getPhysicsManager()));
	outEntityFactories.push_back(gc_new< PathEntityFactory >());
}

void AnimationEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< PathEntityRenderer >());
}

scene::IEntityEditor* AnimationEditorProfile::createEntityEditor(
	scene::SceneEditorContext* context,
	const Type& entityDataType
) const
{
	if (is_type_of< AnimatedMeshEntityData >(entityDataType))
		return gc_new< AnimatedMeshEntityEditor >();
	if (is_type_of< PathEntityData >(entityDataType))
		return gc_new< PathEntityEditor >();
	return 0;
}

	}
}
