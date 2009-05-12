#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

MemberArray::MemberArray(const std::wstring& name)
:	m_name(name)
{
}

const std::wstring& MemberArray::getName() const
{
	return m_name;
}

const Type* MemberArray::getType() const
{
	return 0;
}

bool MemberArray::serialize(Serializer& s, size_t index) const
{
	bool result = false;

	if (s.getDirection() == Serializer::SdRead)
		result = read(s);

	if (s.getDirection() == Serializer::SdWrite)
		result = write(s, index);

	return result;
}

}
