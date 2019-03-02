#include "Database/Remote/Messages/MsgStringResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgStringResult", 0, MsgStringResult, IMessage)

MsgStringResult::MsgStringResult(const std::wstring& value)
:	m_value(value)
{
}

void MsgStringResult::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"value", m_value);
}

	}
}
