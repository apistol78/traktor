#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/SwfMembers.h"

namespace traktor
{
	namespace spark
	{

MemberColorTransform::MemberColorTransform(const wchar_t* const name, ColorTransform& ref)
:	MemberComplex(name, true)
,	m_ref(ref)
{
}

void MemberColorTransform::serialize(ISerializer& s) const
{
	s >> Member< Color4f >(L"mul", m_ref.mul);
	s >> Member< Color4f >(L"add", m_ref.add);
}

	}
}
