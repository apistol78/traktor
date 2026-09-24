/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/Traverser.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/Serializer.h"
#include "World/EntityData.h"

namespace traktor::world
{
namespace
{

/*! Collect direct child object references of an object, in member order.
 *
 * Same set, and order, of objects as RfmObject members found through
 * Reflection but without creating a reflection member for every member
 * of the object.
 */
class ChildObjectSerializer : public Serializer
{
public:
	explicit ChildObjectSerializer(AlignedVector< ISerializable* >& outChildren)
		: m_children(outChildren)
	{
	}

	void collect(const ISerializable* object)
	{
		serialize(const_cast< ISerializable* >(object));
	}

	virtual Direction getDirection() const override final { return Direction::Write; }

	virtual void operator>>(const Member< bool >& m) override final {}

	virtual void operator>>(const Member< int8_t >& m) override final {}

	virtual void operator>>(const Member< uint8_t >& m) override final {}

	virtual void operator>>(const Member< int16_t >& m) override final {}

	virtual void operator>>(const Member< uint16_t >& m) override final {}

	virtual void operator>>(const Member< int32_t >& m) override final {}

	virtual void operator>>(const Member< uint32_t >& m) override final {}

	virtual void operator>>(const Member< int64_t >& m) override final {}

	virtual void operator>>(const Member< uint64_t >& m) override final {}

	virtual void operator>>(const Member< float >& m) override final {}

	virtual void operator>>(const Member< double >& m) override final {}

	virtual void operator>>(const Member< std::string >& m) override final {}

	virtual void operator>>(const Member< std::wstring >& m) override final {}

	virtual void operator>>(const Member< Guid >& m) override final {}

	virtual void operator>>(const Member< Path >& m) override final {}

	virtual void operator>>(const Member< Color4ub >& m) override final {}

	virtual void operator>>(const Member< Color4f >& m) override final {}

	virtual void operator>>(const Member< Scalar >& m) override final {}

	virtual void operator>>(const Member< Vector2 >& m) override final {}

	virtual void operator>>(const Member< Vector4 >& m) override final {}

	virtual void operator>>(const Member< Matrix33 >& m) override final {}

	virtual void operator>>(const Member< Matrix44 >& m) override final {}

	virtual void operator>>(const Member< Quaternion >& m) override final {}

	virtual void operator>>(const Member< ISerializable* >& m) override final
	{
		if (m.getValue())
			m_children.push_back(m.getValue());
	}

	virtual void operator>>(const Member< void* >& m) override final {}

	virtual void operator>>(const MemberArray& m) override final
	{
		m.write(*this, m.size());
	}

	virtual void operator>>(const MemberComplex& m) override final
	{
		m.serialize(*this);
	}

	virtual void operator>>(const MemberEnumBase& m) override final {}

private:
	AlignedVector< ISerializable* >& m_children;
};

}

bool Traverser::visit(const ISerializable* object, const std::function< Result(const EntityData*) >& visitor)
{
	if (!object)
		return true;

	AlignedVector< ISerializable* > children;
	ChildObjectSerializer(children).collect(object);

	for (auto child : children)
	{
		if (auto entityData = dynamic_type_cast< const EntityData* >(child))
		{
			const Result result = visitor(entityData);
			if (result == Result::Continue)
			{
				if (!Traverser::visit(entityData, visitor))
					return false;
			}
			else if (result == Result::Failed)
				return false;
		}
		else
		{
			if (!Traverser::visit((const ISerializable*)child, visitor))
				return false;
		}
	}

	return true;
}

bool Traverser::visit(ISerializable* object, const std::function< Result(Ref< EntityData >&) >& visitor)
{
	if (!object)
		return true;

	AlignedVector< ISerializable* > children;
	ChildObjectSerializer(children).collect(object);

	// Replacements are rare; only reflect the object when a visitor has replaced a child entity.
	AlignedVector< std::pair< ISerializable*, Ref< EntityData > > > replaced;

	for (auto child : children)
	{
		Ref< EntityData > entityData = dynamic_type_cast< world::EntityData* >(child);
		if (entityData)
		{
			const Result result = visitor(entityData);
			if (result == Result::Continue)
			{
				if (!Traverser::visit(entityData, visitor))
					return false;
			}
			else if (result == Result::Failed)
				return false;

			if (entityData != child)
				replaced.push_back({ child, entityData });
		}
		else
		{
			if (!Traverser::visit(child, visitor))
				return false;
		}
	}

	if (!replaced.empty())
	{
		Ref< Reflection > reflection = Reflection::create(object);
		if (!reflection)
			return false;

		RefArray< ReflectionMember > objectMembers;
		reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

		// Members are in the same order as collected children; match each replacement in turn.
		auto it = replaced.begin();
		for (auto member : objectMembers)
		{
			if (it == replaced.end())
				break;

			RfmObject* objectMember = mandatory_non_null_type_cast< RfmObject* >(member.ptr());
			if (objectMember->get() == it->first)
			{
				objectMember->set(it->second);
				++it;
			}
		}

		reflection->apply(object);
	}

	return true;
}

}
