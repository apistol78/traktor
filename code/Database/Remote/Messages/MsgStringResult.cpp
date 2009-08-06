#include "Database/Remote/Messages/MsgStringResult.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.MsgStringResult", MsgStringResult, IMessage)

MsgStringResult::MsgStringResult(const std::wstring& value)
:	m_value(value)
{
}

bool MsgStringResult::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"value", m_value);
}

	}
}
