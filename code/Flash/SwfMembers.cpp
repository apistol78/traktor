#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

MemberSwfCxTransform::MemberSwfCxTransform(const wchar_t* const name, SwfCxTransform& ref)
:	MemberComplex(name, true)
,	m_ref(ref)
{
}

void MemberSwfCxTransform::serialize(ISerializer& s) const
{
	s >> Member< Color4f >(L"mul", m_ref.mul);
	s >> Member< Color4f >(L"add", m_ref.add);
}

	}
}
