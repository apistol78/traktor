#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidEntityData", 0, SolidEntityData, world::GroupEntityData)

SolidEntityData::SolidEntityData()
:	m_outputGuid(Guid::create())
{
}

Ref< SolidEntity > SolidEntityData::createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

    Ref< SolidEntity > solidEntity = new SolidEntity(renderSystem, shader, getTransform());

    for (auto childEntityData : getEntityData())
    {
        Ref< PrimitiveEntity > primitiveEntity = builder->create< PrimitiveEntity >(childEntityData);
        if (primitiveEntity)
            solidEntity->addEntity(primitiveEntity);
    }

	return solidEntity;
}

void SolidEntityData::setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SolidEntityData::getCollisionGroup() const
{
	return m_collisionGroup;
}

void SolidEntityData::setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SolidEntityData::getCollisionMask() const
{
	return m_collisionMask;
}

void SolidEntityData::serialize(ISerializer& s)
{
	world::GroupEntityData::serialize(s);

	s >> Member< Guid >(L"outputGuid", m_outputGuid, AttributePrivate());
	s >> resource::Member< render::Shader >(L"shader", m_shader);
    s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
    s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
}

    }
}