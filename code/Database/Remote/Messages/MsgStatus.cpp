#include "Database/Remote/Messages/MsgStatus.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.MsgStatus", MsgStatus, IMessage)

MsgStatus::MsgStatus(MsgStatusType status)
:	m_status(int32_t(status))
{
}

bool MsgStatus::serialize(Serializer& s)
{
	return s >> Member< int32_t >(L"status", m_status);
}

	}
}
