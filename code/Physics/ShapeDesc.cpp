#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/ShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc, ISerializable)

ShapeDesc::ShapeDesc()
:	m_localTransform(Transform::identity())
,	m_material(0)
{
}

void ShapeDesc::setLocalTransform(const Transform& localTransform)
{
	m_localTransform = localTransform;
}

const Transform& ShapeDesc::getLocalTransform() const
{
	return m_localTransform;
}

void ShapeDesc::setCollisionGroup(const std::set< resource::Id< CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const std::set< resource::Id< CollisionSpecification > >& ShapeDesc::getCollisionGroup() const
{
	return m_collisionGroup;
}

void ShapeDesc::setCollisionMask(const std::set< resource::Id< CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const std::set< resource::Id< CollisionSpecification > >& ShapeDesc::getCollisionMask() const
{
	return m_collisionMask;
}

void ShapeDesc::setMaterial(int32_t material)
{
	m_material = material;
}

int32_t ShapeDesc::getMaterial() const
{
	return m_material;
}

void ShapeDesc::serialize(ISerializer& s)
{
	s >> MemberComposite< Transform >(L"localTransform", m_localTransform);

	if (s.getVersion() >= 5)
	{
		s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
		s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"collisionMask", m_collisionMask);
	}
	else
	{
		uint32_t collisionGroup, collisionMask;
		s >> Member< uint32_t >(L"collisionGroup", collisionGroup);
		s >> Member< uint32_t >(L"collisionMask", collisionMask);

		const struct
		{
			uint32_t v;
			Guid g;
		}
		c_setup[] =
		{
			{ 0x0001, Guid(L"{338B52C0-B01B-464A-90F8-EA9883CBA476}") },
			{ 0x0002, Guid(L"{BA1AA800-D964-4948-A972-ACB18FBF4172}") },
			{ 0x0004, Guid(L"{20E6CBBF-58BE-B644-923C-A1F897CB28E5}") },
			{ 0x0008, Guid(L"{443C89C9-2724-A641-965B-96E4676B5199}") },
			{ 0x0010, Guid(L"{DC881305-1CFF-7746-89B1-2F1F82332EF4}") },
			{ 0x0020, Guid(L"{799D2133-65C1-CB43-A7A3-CE996705EDB6}") },
			{ 0x0040, Guid(L"{F7BE3876-17E8-D946-8727-6C501CFED32C}") },
			{ 0x0080, Guid(L"{06F9D948-DBA1-5646-9D84-AAB89DA0F9DE}") },
			{ 0x0100, Guid(L"{107D7A8E-842A-8643-834A-F41521F9B5AE}") },
			{ 0x0200, Guid(L"{7A81E0C2-2FFC-1847-BEDC-6C34FF2806A0}") }
		};
		for (uint32_t i = 0; i < sizeof_array(c_setup); ++i)
		{
			if ((collisionGroup & c_setup[i].v) != 0)
				m_collisionGroup.insert(resource::Id< CollisionSpecification >(c_setup[i].g));
			if ((collisionMask & c_setup[i].v) != 0)
				m_collisionMask.insert(resource::Id< CollisionSpecification >(c_setup[i].g));
		}
	}

	if (s.getVersion() >= 3)
		s >> Member< int32_t >(L"material", m_material);
}

	}
}
