#include "Database/Remote/Messages/StmSeek.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.StmSeek", StmSeek, IMessage)

StmSeek::StmSeek(uint32_t handle, Stream::SeekOriginType origin, int32_t offset)
:	m_handle(handle)
,	m_origin(int32_t(origin))
,	m_offset(offset)
{
}

bool StmSeek::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< int32_t >(L"origin", m_origin);
	s >> Member< int32_t >(L"offset", m_offset);
	return true;
}

	}
}
