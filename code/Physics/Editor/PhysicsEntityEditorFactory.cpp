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

const TypeInfoSet PhysicsEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< RigidEntityData >());
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > PhysicsEntityEditorFactory::createEntityEditor(
	scene::SceneEditorContext* context,
	const TypeInfo& entityDataType
) const
{
	if (is_type_of< RigidEntityData >(entityDataType))
		return new RigidEntityEditor(context);
	if (is_type_of< ArticulatedEntityEditor >(entityDataType))
		return new ArticulatedEntityEditor(context);
	return 0;
}

	}
}
