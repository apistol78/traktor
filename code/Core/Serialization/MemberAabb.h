#ifndef traktor_MemberAabb_H
#define traktor_MemberAabb_H

#include "Core/Math/Aabb3.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

class MemberAabb : public MemberComplex
{
public:
	MemberAabb(const wchar_t* const name, Aabb3& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< Vector4 >(L"mn", m_ref.mn, AttributePoint());
		s >> Member< Vector4 >(L"mx", m_ref.mx, AttributePoint());
	}

private:
	Aabb3& m_ref;
};

}

#endif	// traktor_MemberAabb_H
