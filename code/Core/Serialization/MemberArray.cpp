#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/ISerializer.h"

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

bool MemberArray::serialize(ISerializer& s, size_t index) const
{
	bool result = false;

	if (s.getDirection() == ISerializer::SdRead)
		result = read(s);

	if (s.getDirection() == ISerializer::SdWrite)
		result = write(s, index);

	return result;
}

}
