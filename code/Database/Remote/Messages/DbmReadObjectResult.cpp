#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmReadObjectResult", 0, DbmReadObjectResult, IMessage)

DbmReadObjectResult::DbmReadObjectResult(uint32_t streamId, const std::wstring& serializerTypeName)
:	m_streamId(streamId)
,	m_serializerTypeName(serializerTypeName)
{
}

bool DbmReadObjectResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"streamId", m_streamId);
	s >> Member< std::wstring >(L"serializerTypeName", m_serializerTypeName);
	return true;
}

	}
}
