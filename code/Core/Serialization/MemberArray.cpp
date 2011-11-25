#include "Core/Serialization/MemberArray.h"

namespace traktor
{

MemberArray::MemberArray(const wchar_t* const name)
:	m_name(name)
{
}

MemberArray::~MemberArray()
{
}

const wchar_t* const MemberArray::getName() const
{
	return m_name;
}

const TypeInfo* MemberArray::getType() const
{
	return 0;
}

}
