#include "Core/Reflection/ReflectionInspectSerializer.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ReflectionInspectSerializer", ReflectionInspectSerializer, Serializer)

ReflectionInspectSerializer::ReflectionInspectSerializer(RfmCompound* compound)
:	m_compoundMember(compound)
{
}

Serializer::Direction ReflectionInspectSerializer::getDirection() const
{
	return SdWrite;
}

void ReflectionInspectSerializer::operator >> (const Member< bool >& m)
{
	addMember(new RfmPrimitiveBoolean(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< int8_t >& m)
{
	addMember(new RfmPrimitiveInt8(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< uint8_t >& m)
{
	addMember(new RfmPrimitiveUInt8(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< int16_t >& m)
{
	addMember(new RfmPrimitiveInt16(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< uint16_t >& m)
{
	addMember(new RfmPrimitiveUInt16(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< int32_t >& m)
{
	addMember(new RfmPrimitiveInt32(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< uint32_t >& m)
{
	addMember(new RfmPrimitiveUInt32(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< int64_t >& m)
{
	addMember(new RfmPrimitiveInt64(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< uint64_t >& m)
{
	addMember(new RfmPrimitiveUInt64(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< float >& m)
{
	addMember(new RfmPrimitiveFloat(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< double >& m)
{
	addMember(new RfmPrimitiveDouble(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< std::string >& m)
{
	addMember(new RfmPrimitiveString(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< std::wstring >& m)
{
	addMember(new RfmPrimitiveWideString(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Guid >& m)
{
	addMember(new RfmPrimitiveGuid(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Path >& m)
{
	addMember(new RfmPrimitivePath(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Color4ub >& m)
{
	addMember(new RfmPrimitiveColor4ub(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Color4f >& m)
{
	addMember(new RfmPrimitiveColor4f(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Scalar >& m)
{
	addMember(new RfmPrimitiveScalar(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Vector2 >& m)
{
	addMember(new RfmPrimitiveVector2(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Vector4 >& m)
{
	addMember(new RfmPrimitiveVector4(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Matrix33 >& m)
{
	addMember(new RfmPrimitiveMatrix33(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Matrix44 >& m)
{
	addMember(new RfmPrimitiveMatrix44(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< Quaternion >& m)
{
	addMember(new RfmPrimitiveQuaternion(m.getName(), m));
}

void ReflectionInspectSerializer::operator >> (const Member< ISerializable* >& m)
{
	Ref< ISerializable > object = m;
	addMember(new RfmObject(m.getName(), object));
}

void ReflectionInspectSerializer::operator >> (const Member< void* >& m)
{
	T_FATAL_ERROR;
}

void ReflectionInspectSerializer::operator >> (const MemberArray& m)
{
	Ref< RfmArray > arrayMember = new RfmArray(m.getName());

	Ref< RfmCompound > currentCompoundMember = m_compoundMember;
	m_compoundMember = arrayMember;

	for (size_t i = 0; i < m.size(); ++i)
		m.write(*this);

	m_compoundMember = currentCompoundMember;

	addMember(arrayMember);
}

void ReflectionInspectSerializer::operator >> (const MemberComplex& m)
{
	Ref< RfmCompound > currentCompoundMember = m_compoundMember;
	Ref< RfmCompound > compoundMember;

	if (m.getCompound())
	{
		compoundMember = new RfmCompound(m.getName());
		m_compoundMember = compoundMember;
	}

	m.serialize(*this);

	if (m.getCompound())
	{
		m_compoundMember = currentCompoundMember;
		addMember(compoundMember);
	}
}

void ReflectionInspectSerializer::operator >> (const MemberEnumBase& m)
{
	addMember(new RfmEnum(m.getName(), m.get()));
}

bool ReflectionInspectSerializer::addMember(ReflectionMember* member)
{
	if (m_compoundMember)
	{
		m_compoundMember->addMember(member);
		return true;
	}
	else
		return false;
}

}
