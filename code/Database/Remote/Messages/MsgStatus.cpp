#include "Database/Remote/Messages/MsgStatus.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgStatus", 0, MsgStatus, IMessage)

MsgStatus::MsgStatus(MsgStatusType status)
:	m_status(int32_t(status))
{
}

void MsgStatus::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"status", m_status);
}

	}
}
