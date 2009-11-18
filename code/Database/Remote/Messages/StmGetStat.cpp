#include "Database/Remote/Messages/StmGetStat.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmGetStat", 0, StmGetStat, IMessage)

StmGetStat::StmGetStat(uint32_t handle)
:	m_handle(handle)
{
}

bool StmGetStat::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
