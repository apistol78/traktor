#include "Database/Remote/Messages/DbmWriteObject.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmWriteObject", DbmWriteObject, IMessage)

DbmWriteObject::DbmWriteObject(uint32_t handle, const std::wstring& primaryTypeName)
:	m_handle(handle)
,	m_primaryTypeName(primaryTypeName)
{
}

bool DbmWriteObject::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"primaryTypeName", m_primaryTypeName);
	return true;
}

	}
}
