#include <string>
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

MemberComplex::MemberComplex(const wchar_t* const name, bool compound)
:	m_name(name)
,	m_compound(compound)
{
}

MemberComplex::~MemberComplex()
{
}

const wchar_t* const MemberComplex::getName() const
{
	return m_name;
}

bool MemberComplex::getCompound() const
{
	return m_compound;
}

}
