#include "Animation/AnimatedMeshEntityFactory.h"
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/AnimatedMeshEntity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshEntityFactory", AnimatedMeshEntityFactory, world::IEntityFactory)

AnimatedMeshEntityFactory::AnimatedMeshEntityFactory(physics::PhysicsManager* physicsManager)
:	m_physicsManager(physicsManager)
{
}

const TypeSet AnimatedMeshEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	return typeSet;
}

world::Entity* AnimatedMeshEntityFactory::createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData) const
{
	return checked_type_cast< const AnimatedMeshEntityData* >(&entityData)->createEntity(m_physicsManager);
}

	}
}
