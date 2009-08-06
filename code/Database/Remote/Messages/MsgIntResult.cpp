#include "Database/Remote/Messages/MsgIntResult.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.MsgIntResult", MsgIntResult, IMessage)

MsgIntResult::MsgIntResult(int32_t value)
:	m_value(value)
{
}

bool MsgIntResult::serialize(Serializer& s)
{
	return s >> Member< uint32_t >(L"value", m_value);
}

	}
}
