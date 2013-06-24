#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

MemberSwfColor::MemberSwfColor(const wchar_t* const name, SwfColor& ref)
:	MemberComplex(name, true)
,	m_ref(ref)
{
}

void MemberSwfColor::serialize(ISerializer& s) const
{
	s >> Member< uint8_t >(L"red", m_ref.red);
	s >> Member< uint8_t >(L"green", m_ref.green);
	s >> Member< uint8_t >(L"blue", m_ref.blue);
	s >> Member< uint8_t >(L"alpha", m_ref.alpha);
}

MemberSwfCxTransform::MemberSwfCxTransform(const wchar_t* const name, SwfCxTransform& ref)
:	MemberComplex(name, true)
,	m_ref(ref)
{
}

void MemberSwfCxTransform::serialize(ISerializer& s) const
{
	s >> MemberStaticArray< float, 2 >(L"red", m_ref.red);
	s >> MemberStaticArray< float, 2 >(L"green", m_ref.green);
	s >> MemberStaticArray< float, 2 >(L"blue", m_ref.blue);
	s >> MemberStaticArray< float, 2 >(L"alpha", m_ref.alpha);
}

	}
}
