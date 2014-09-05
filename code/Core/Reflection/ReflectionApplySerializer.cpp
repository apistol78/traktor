#include "Core/Reflection/ReflectionApplySerializer.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ReflectionApplySerializer", ReflectionApplySerializer, Serializer)

ReflectionApplySerializer::ReflectionApplySerializer(const RfmCompound* compound)
:	m_compoundMember(compound)
,	m_memberIndex(0)
{
}

Serializer::Direction ReflectionApplySerializer::getDirection() const
{
	return SdRead;
}

void ReflectionApplySerializer::operator >> (const Member< bool >& m)
{
	Ref< const RfmPrimitiveBoolean > member = dynamic_type_cast< const RfmPrimitiveBoolean* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int8_t >& m)
{
	Ref< const RfmPrimitiveInt8 > member = dynamic_type_cast< const RfmPrimitiveInt8* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint8_t >& m)
{
	Ref< const RfmPrimitiveUInt8 > member = dynamic_type_cast< const RfmPrimitiveUInt8* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int16_t >& m)
{
	Ref< const RfmPrimitiveInt16 > member = dynamic_type_cast< const RfmPrimitiveInt16* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint16_t >& m)
{
	Ref< const RfmPrimitiveUInt16 > member = dynamic_type_cast< const RfmPrimitiveUInt16* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int32_t >& m)
{
	Ref< const RfmPrimitiveInt32 > member = dynamic_type_cast< const RfmPrimitiveInt32* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint32_t >& m)
{
	Ref< const RfmPrimitiveUInt32 > member = dynamic_type_cast< const RfmPrimitiveUInt32* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int64_t >& m)
{
	Ref< const RfmPrimitiveInt64 > member = dynamic_type_cast< const RfmPrimitiveInt64* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint64_t >& m)
{
	Ref< const RfmPrimitiveUInt64 > member = dynamic_type_cast< const RfmPrimitiveUInt64* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< float >& m)
{
	Ref< const RfmPrimitiveFloat > member = dynamic_type_cast< const RfmPrimitiveFloat* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< double >& m)
{
	Ref< const RfmPrimitiveDouble > member = dynamic_type_cast< const RfmPrimitiveDouble* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< std::string >& m)
{
	Ref< const RfmPrimitiveString > member = dynamic_type_cast< const RfmPrimitiveString* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< std::wstring >& m)
{
	Ref< const RfmPrimitiveWideString > member = dynamic_type_cast< const RfmPrimitiveWideString* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Guid >& m)
{
	Ref< const RfmPrimitiveGuid > member = dynamic_type_cast< const RfmPrimitiveGuid* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Path >& m)
{
	Ref< const RfmPrimitivePath > member = dynamic_type_cast< const RfmPrimitivePath* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Color4f >& m)
{
	Ref< const RfmPrimitiveColor4f > member = dynamic_type_cast< const RfmPrimitiveColor4f* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Color4ub >& m)
{
	Ref< const RfmPrimitiveColor4ub > member = dynamic_type_cast< const RfmPrimitiveColor4ub* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Scalar >& m)
{
	Ref< const RfmPrimitiveScalar > member = dynamic_type_cast< const RfmPrimitiveScalar* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Vector2 >& m)
{
	Ref< const RfmPrimitiveVector2 > member = dynamic_type_cast< const RfmPrimitiveVector2* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Vector4 >& m)
{
	Ref< const RfmPrimitiveVector4 > member = dynamic_type_cast< const RfmPrimitiveVector4* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Matrix33 >& m)
{
	Ref< const RfmPrimitiveMatrix33 > member = dynamic_type_cast< const RfmPrimitiveMatrix33* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Matrix44 >& m)
{
	Ref< const RfmPrimitiveMatrix44 > member = dynamic_type_cast< const RfmPrimitiveMatrix44* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Quaternion >& m)
{
	Ref< const RfmPrimitiveQuaternion > member = dynamic_type_cast< const RfmPrimitiveQuaternion* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< ISerializable* >& m)
{
	Ref< const RfmObject > member = dynamic_type_cast< const RfmObject* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< void* >& m)
{
}

void ReflectionApplySerializer::operator >> (const MemberArray& m)
{
	Ref< const RfmArray > arrayMember = dynamic_type_cast< const RfmArray* >(getNextMember());
	if (!arrayMember)
		return;

	uint32_t currentMemberIndex = m_memberIndex;
	Ref< const RfmCompound > currentCompoundMember = m_compoundMember;
	
	m_memberIndex = 0;
	m_compoundMember = arrayMember;

	m.reserve(
		0,
		m_compoundMember->getMemberCount()
	);

	for (uint32_t i = 0; i < m_compoundMember->getMemberCount(); ++i)
		m.read(*this);

	T_ASSERT (m_memberIndex == m_compoundMember->getMemberCount());

	m_memberIndex = currentMemberIndex;
	m_compoundMember = currentCompoundMember;
}

void ReflectionApplySerializer::operator >> (const MemberComplex& m)
{
	uint32_t currentMemberIndex;
	Ref< const RfmCompound > currentCompoundMember;

	if (m.getCompound())
	{
		Ref< const RfmCompound > compoundMember = dynamic_type_cast< const RfmCompound* >(getNextMember());
		if (!compoundMember)
			return;

		currentMemberIndex = m_memberIndex;
		currentCompoundMember = m_compoundMember;

		m_memberIndex = 0;
		m_compoundMember = compoundMember;
	}

	m.serialize(*this);

	if (m.getCompound())
	{
		m_memberIndex = currentMemberIndex;
		m_compoundMember = currentCompoundMember;
	}
}

void ReflectionApplySerializer::operator >> (const MemberEnumBase& m)
{
	Ref< const RfmEnum > member = dynamic_type_cast< const RfmEnum* >(getNextMember());
	if (member)
		m.set(member->get());
}

Ref< const ReflectionMember > ReflectionApplySerializer::getNextMember()
{
	if (m_compoundMember)
	{
		T_ASSERT (m_memberIndex < m_compoundMember->getMemberCount());
		return m_compoundMember->getMember(m_memberIndex++);
	}
	else
		return 0;
}

}
