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

bool ReflectionInspectSerializer::operator >> (const Member< bool >& m)
{
	return addMember(new RfmPrimitive< bool >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< int8_t >& m)
{
	return addMember(new RfmPrimitive< int8_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< uint8_t >& m)
{
	return addMember(new RfmPrimitive< uint8_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< int16_t >& m)
{
	return addMember(new RfmPrimitive< int16_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< uint16_t >& m)
{
	return addMember(new RfmPrimitive< uint16_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< int32_t >& m)
{
	return addMember(new RfmPrimitive< int32_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< uint32_t >& m)
{
	return addMember(new RfmPrimitive< uint32_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< int64_t >& m)
{
	return addMember(new RfmPrimitive< int64_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< uint64_t >& m)
{
	return addMember(new RfmPrimitive< uint64_t >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< float >& m)
{
	return addMember(new RfmPrimitive< float >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< double >& m)
{
	return addMember(new RfmPrimitive< double >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< std::string >& m)
{
	return addMember(new RfmPrimitive< std::string >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< std::wstring >& m)
{
	return addMember(new RfmPrimitive< std::wstring >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Guid >& m)
{
	return addMember(new RfmPrimitive< Guid >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Path >& m)
{
	return addMember(new RfmPrimitive< Path >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Color4ub >& m)
{
	return addMember(new RfmPrimitive< Color4ub >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Color4f >& m)
{
	return addMember(new RfmPrimitive< Color4f >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Scalar >& m)
{
	return addMember(new RfmPrimitive< Scalar >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Vector2 >& m)
{
	return addMember(new RfmPrimitive< Vector2 >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Vector4 >& m)
{
	return addMember(new RfmPrimitive< Vector4 >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Matrix33 >& m)
{
	return addMember(new RfmPrimitive< Matrix33 >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Matrix44 >& m)
{
	return addMember(new RfmPrimitive< Matrix44 >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< Quaternion >& m)
{
	return addMember(new RfmPrimitive< Quaternion >(m.getName(), m));
}

bool ReflectionInspectSerializer::operator >> (const Member< ISerializable* >& m)
{
	Ref< ISerializable > object = m;
	return addMember(new RfmObject(m.getName(), object));
}

bool ReflectionInspectSerializer::operator >> (const Member< void* >& m)
{
	T_FATAL_ERROR;
	return false;
}

bool ReflectionInspectSerializer::operator >> (const MemberArray& m)
{
	Ref< RfmArray > arrayMember = new RfmArray(m.getName());

	Ref< RfmCompound > currentCompoundMember = m_compoundMember;
	m_compoundMember = arrayMember;

	for (size_t i = 0; i < m.size(); ++i)
		m.write(*this);

	m_compoundMember = currentCompoundMember;

	return addMember(arrayMember);
}

bool ReflectionInspectSerializer::operator >> (const MemberComplex& m)
{
	Ref< RfmCompound > compoundMember = new RfmCompound(m.getName());

	Ref< RfmCompound > currentCompoundMember = m_compoundMember;
	m_compoundMember = compoundMember;

	if (!m.serialize(*this))
		return false;

	m_compoundMember = currentCompoundMember;

	return addMember(compoundMember);
}

bool ReflectionInspectSerializer::operator >> (const MemberEnumBase& m)
{
	return addMember(new RfmEnum(m.getName(), m.get()));
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
