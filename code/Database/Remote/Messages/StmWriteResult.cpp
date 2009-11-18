#include "Database/Remote/Messages/StmWriteResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmWriteResult", 0, StmWriteResult, IMessage)

StmWriteResult::StmWriteResult(int32_t result)
:	m_result(result)
{
}

bool StmWriteResult::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"result", m_result);
}

	}
}
