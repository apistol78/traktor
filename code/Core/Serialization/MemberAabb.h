#ifndef traktor_MemberAabb_H
#define traktor_MemberAabb_H

#include "Core/Math/Aabb3.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

class MemberAabb : public MemberComplex
{
public:
	MemberAabb(const std::wstring& name, Aabb3& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (!(s >> Member< Vector4 >(L"mn", m_ref.mn)))
			return false;
		if (!(s >> Member< Vector4 >(L"mx", m_ref.mx)))
			return false;
		return true;
	}

private:
	Aabb3& m_ref;
};

}

#endif	// traktor_MemberAabb_H
