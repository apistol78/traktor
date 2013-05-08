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
	Ref< const RfmPrimitive< bool > > member = dynamic_type_cast< const RfmPrimitive< bool >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int8_t >& m)
{
	Ref< const RfmPrimitive< int8_t > > member = dynamic_type_cast< const RfmPrimitive< int8_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint8_t >& m)
{
	Ref< const RfmPrimitive< uint8_t > > member = dynamic_type_cast< const RfmPrimitive< uint8_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int16_t >& m)
{
	Ref< const RfmPrimitive< int16_t > > member = dynamic_type_cast< const RfmPrimitive< int16_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint16_t >& m)
{
	Ref< const RfmPrimitive< uint16_t > > member = dynamic_type_cast< const RfmPrimitive< uint16_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int32_t >& m)
{
	Ref< const RfmPrimitive< int32_t > > member = dynamic_type_cast< const RfmPrimitive< int32_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint32_t >& m)
{
	Ref< const RfmPrimitive< uint32_t > > member = dynamic_type_cast< const RfmPrimitive< uint32_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< int64_t >& m)
{
	Ref< const RfmPrimitive< int64_t > > member = dynamic_type_cast< const RfmPrimitive< int64_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< uint64_t >& m)
{
	Ref< const RfmPrimitive< uint64_t > > member = dynamic_type_cast< const RfmPrimitive< uint64_t >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< float >& m)
{
	Ref< const RfmPrimitive< float > > member = dynamic_type_cast< const RfmPrimitive< float >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< double >& m)
{
	Ref< const RfmPrimitive< double > > member = dynamic_type_cast< const RfmPrimitive< double >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< std::string >& m)
{
	Ref< const RfmPrimitive< std::string > > member = dynamic_type_cast< const RfmPrimitive< std::string >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< std::wstring >& m)
{
	Ref< const RfmPrimitive< std::wstring > > member = dynamic_type_cast< const RfmPrimitive< std::wstring >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Guid >& m)
{
	Ref< const RfmPrimitive< Guid > > member = dynamic_type_cast< const RfmPrimitive< Guid >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Path >& m)
{
	Ref< const RfmPrimitive< Path > > member = dynamic_type_cast< const RfmPrimitive< Path >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Color4f >& m)
{
	Ref< const RfmPrimitive< Color4f > > member = dynamic_type_cast< const RfmPrimitive< Color4f >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Color4ub >& m)
{
	Ref< const RfmPrimitive< Color4ub > > member = dynamic_type_cast< const RfmPrimitive< Color4ub >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Scalar >& m)
{
	Ref< const RfmPrimitive< Scalar > > member = dynamic_type_cast< const RfmPrimitive< Scalar >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Vector2 >& m)
{
	Ref< const RfmPrimitive< Vector2 > > member = dynamic_type_cast< const RfmPrimitive< Vector2 >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Vector4 >& m)
{
	Ref< const RfmPrimitive< Vector4 > > member = dynamic_type_cast< const RfmPrimitive< Vector4 >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Matrix33 >& m)
{
	Ref< const RfmPrimitive< Matrix33 > > member = dynamic_type_cast< const RfmPrimitive< Matrix33 >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Matrix44 >& m)
{
	Ref< const RfmPrimitive< Matrix44 > > member = dynamic_type_cast< const RfmPrimitive< Matrix44 >* >(getNextMember());
	if (member)
		m = member->get();
}

void ReflectionApplySerializer::operator >> (const Member< Quaternion >& m)
{
	Ref< const RfmPrimitive< Quaternion > > member = dynamic_type_cast< const RfmPrimitive< Quaternion >* >(getNextMember());
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
	Ref< const RfmCompound > compoundMember = dynamic_type_cast< const RfmCompound* >(getNextMember());
	if (!compoundMember)
		return;

	uint32_t currentMemberIndex = m_memberIndex;
	Ref< const RfmCompound > currentCompoundMember = m_compoundMember;

	m_memberIndex = 0;
	m_compoundMember = compoundMember;

	m.serialize(*this);

	T_ASSERT (m_memberIndex == m_compoundMember->getMemberCount());

	m_memberIndex = currentMemberIndex;
	m_compoundMember = currentCompoundMember;
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
