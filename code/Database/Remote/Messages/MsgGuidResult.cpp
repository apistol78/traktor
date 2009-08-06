#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.MsgGuidResult", MsgGuidResult, IMessage)

MsgGuidResult::MsgGuidResult(const Guid& value)
:	m_value(value)
{
}

bool MsgGuidResult::serialize(Serializer& s)
{
	return s >> Member< Guid >(L"value", m_value);
}

	}
}
