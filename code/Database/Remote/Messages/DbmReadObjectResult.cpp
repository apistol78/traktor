#include "Database/Remote/Messages/DbmReadObjectResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmReadObjectResult", DbmReadObjectResult, IMessage)

DbmReadObjectResult::DbmReadObjectResult(uint32_t handle, const std::wstring& serializerTypeName)
:	m_handle(handle)
,	m_serializerTypeName(serializerTypeName)
{
}

bool DbmReadObjectResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"serializerTypeName", m_serializerTypeName);
	return true;
}

	}
}
