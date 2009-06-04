#include "Physics/Editor/PhysicsEditorProfile.h"
#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/World/EntityFactory.h"
#include "Physics/World/EntityRenderer.h"
#include "Physics/MeshFactory.h"
#include "Physics/HeightfieldFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Heap/New.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.PhysicsEditorProfile", PhysicsEditorProfile, scene::SceneEditorProfile)

void PhysicsEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Physics.ToggleMeshTriangles"));
}

void PhysicsEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::ResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< MeshFactory >(context->getResourceDatabase()));
	outResourceFactories.push_back(gc_new< HeightfieldFactory >(context->getResourceDatabase()));
}

void PhysicsEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::EntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< EntityFactory >(context->getPhysicsManager()));
}

void PhysicsEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::RenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::EntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< EntityRenderer >());
}

void PhysicsEditorProfile::createEntityEditors(
	scene::SceneEditorContext* context,
	RefArray< scene::EntityEditor >& outEntityEditors
) const
{
	outEntityEditors.push_back(gc_new< RigidEntityEditor >());
	outEntityEditors.push_back(gc_new< ArticulatedEntityEditor >());
}

	}
}
