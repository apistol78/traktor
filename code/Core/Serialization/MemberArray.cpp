#include "Core/Serialization/MemberArray.h"

namespace traktor
{

MemberArray::MemberArray(const std::wstring& name)
:	m_name(name)
{
}

MemberArray::~MemberArray()
{
}

const std::wstring& MemberArray::getName() const
{
	return m_name;
}

const TypeInfo* MemberArray::getType() const
{
	return 0;
}

}
