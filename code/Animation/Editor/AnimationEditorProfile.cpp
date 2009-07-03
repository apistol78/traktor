#include "Animation/Editor/AnimationEditorProfile.h"
#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/AnimatedMeshEntityFactory.h"
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.AnimationEditorProfile", AnimationEditorProfile, scene::SceneEditorProfile)

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
	render::RenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< PathEntityRenderer >());
}

void AnimationEditorProfile::createEntityEditors(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditor >& outEntityEditors
) const
{
	outEntityEditors.push_back(gc_new< AnimatedMeshEntityEditor >());
	outEntityEditors.push_back(gc_new< PathEntityEditor >());
}

	}
}
