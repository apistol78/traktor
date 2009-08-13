#include "Physics/Editor/PhysicsEntityEditorFactory.h"
#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/World/RigidEntityData.h"
#include "Physics/World/ArticulatedEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsEntityEditorFactory", PhysicsEntityEditorFactory, scene::IEntityEditorFactory)

const TypeSet PhysicsEntityEditorFactory::getEntityDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

scene::IEntityEditor* PhysicsEntityEditorFactory::createEntityEditor(
	scene::SceneEditorContext* context,
	const Type& entityDataType
) const
{
	if (is_type_of< RigidEntityData >(entityDataType))
		return gc_new< RigidEntityEditor >();
	if (is_type_of< ArticulatedEntityEditor >(entityDataType))
		return gc_new< ArticulatedEntityEditor >();
	return 0;
}

	}
}
