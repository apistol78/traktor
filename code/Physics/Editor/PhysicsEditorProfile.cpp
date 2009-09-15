#include "Physics/Editor/PhysicsEditorProfile.h"
#include "Physics/Editor/PhysicsEntityEditorFactory.h"
#include "Physics/World/EntityFactory.h"
#include "Physics/World/EntityRenderer.h"
#include "Physics/MeshFactory.h"
#include "Physics/HeightfieldFactory.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "Core/Heap/GcNew.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.PhysicsEditorProfile", PhysicsEditorProfile, scene::ISceneEditorProfile)

void PhysicsEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Physics.ToggleMeshTriangles"));
}

void PhysicsEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void PhysicsEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(gc_new< MeshFactory >(context->getResourceDatabase()));
	outResourceFactories.push_back(gc_new< HeightfieldFactory >(context->getResourceDatabase()));
}

void PhysicsEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(gc_new< EntityFactory >(context->getPhysicsManager()));
}

void PhysicsEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(gc_new< EntityRenderer >());
}

void PhysicsEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void PhysicsEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(gc_new< PhysicsEntityEditorFactory >());
}

	}
}
