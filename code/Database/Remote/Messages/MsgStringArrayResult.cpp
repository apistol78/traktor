#include "Database/Remote/Messages/MsgStringArrayResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgStringArrayResult", 0, MsgStringArrayResult, IMessage)

MsgStringArrayResult::MsgStringArrayResult()
{
}

MsgStringArrayResult::MsgStringArrayResult(const std::vector< std::wstring >& values)
:	m_values(values)
{
}

void MsgStringArrayResult::add(const std::wstring& value)
{
	m_values.push_back(value);
}

uint32_t MsgStringArrayResult::count()
{
	return uint32_t(m_values.size());
}

const std::wstring& MsgStringArrayResult::get(uint32_t index) const
{
	return m_values[index];
}

void MsgStringArrayResult::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::wstring >(L"handles", m_values);
}

	}
}
