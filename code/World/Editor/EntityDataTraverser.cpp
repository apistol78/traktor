#include "Core/Serialization/Serializer.h"
#include "World/Editor/EntityDataTraverser.h"
#include "World/Entity/EntityData.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

class TraverseSerializer : public Serializer
{
public:
	TraverseSerializer(EntityDataTraverser::IVisitor& visitor);

	virtual Direction getDirection() const { return SdWrite; }

	virtual bool operator >> (const Member< bool >& m) { return true; }

	virtual bool operator >> (const Member< int8_t >& m) { return true; }

	virtual bool operator >> (const Member< uint8_t >& m) { return true; }

	virtual bool operator >> (const Member< int16_t >& m) { return true; }

	virtual bool operator >> (const Member< uint16_t >& m) { return true; }

	virtual bool operator >> (const Member< int32_t >& m) { return true; }

	virtual bool operator >> (const Member< uint32_t >& m) { return true; }

	virtual bool operator >> (const Member< int64_t >& m) { return true; }

	virtual bool operator >> (const Member< uint64_t >& m) { return true; }

	virtual bool operator >> (const Member< float >& m) { return true; }

	virtual bool operator >> (const Member< double >& m) { return true; }

	virtual bool operator >> (const Member< std::string >& m) { return true; }

	virtual bool operator >> (const Member< std::wstring >& m) { return true; }

	virtual bool operator >> (const Member< Guid >& m) { return true; }

	virtual bool operator >> (const Member< Path >& m) { return true; }

	virtual bool operator >> (const Member< Color4ub >& m) { return true; }

	virtual bool operator >> (const Member< Scalar >& m) { return true; }

	virtual bool operator >> (const Member< Vector2 >& m) { return true; }

	virtual bool operator >> (const Member< Vector4 >& m) { return true; }

	virtual bool operator >> (const Member< Matrix33 >& m) { return true; }

	virtual bool operator >> (const Member< Matrix44 >& m) { return true; }

	virtual bool operator >> (const Member< Quaternion >& m) { return true; }

	virtual bool operator >> (const Member< ISerializable >& m);

	virtual bool operator >> (const Member< ISerializable* >& m);

	virtual bool operator >> (const Member< void* >& m) { return true; }

	virtual bool operator >> (const MemberArray& m);

	virtual bool operator >> (const MemberComplex& m);

	virtual bool operator >> (const MemberEnumBase& m) { return true; }

private:
	EntityDataTraverser::IVisitor& m_visitor;
};

TraverseSerializer::TraverseSerializer(EntityDataTraverser::IVisitor& visitor)
:	m_visitor(visitor)
{
}

bool TraverseSerializer::operator >> (const Member< ISerializable >& m)
{
	return m->serialize(*this);
}

bool TraverseSerializer::operator >> (const Member< ISerializable* >& m)
{
	ISerializable* object = *m;
	if (object)
	{
		int32_t version = type_of(object).getVersion();
		if (EntityData* entityData = dynamic_type_cast< EntityData* >(object))
		{
			Ref< EntityData > entityDataSubst;
			EntityDataTraverser::VisitorResult result;

			result = m_visitor.enter(m.getName(), entityData, entityDataSubst);
			if (result == EntityDataTraverser::VrDefault)
			{
				if (!serialize(object, version, 0))
					return false;

				result = m_visitor.leave(m.getName(), entityData, entityDataSubst);
			}

			if (result == EntityDataTraverser::VrReplace)
				m = entityDataSubst;
		}
		else
		{
			if (!serialize(object, version, 0))
				return false;
		}
	}
	return true;
}

bool TraverseSerializer::operator >> (const MemberArray& m)
{
	size_t size = m.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (!m.write(*this))
			return false;
	}
	return true;
}

bool TraverseSerializer::operator >> (const MemberComplex& m)
{
	return m.serialize(*this);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityDataTraverser", EntityDataTraverser, Object)

EntityDataTraverser::EntityDataTraverser(const EntityData* entityData)
:	m_entityData(entityData)
{
}

void EntityDataTraverser::visit(IVisitor& visitor)
{
	if (m_entityData)
	{
		TraverseSerializer s(visitor);
		const_cast< EntityData* >(m_entityData.c_ptr())->serialize(s);
	}
}

	}
}
