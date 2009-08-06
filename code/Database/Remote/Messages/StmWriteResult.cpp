#include "Database/Remote/Messages/StmWriteResult.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.StmWriteResult", StmWriteResult, IMessage)

StmWriteResult::StmWriteResult(int32_t result)
:	m_result(result)
{
}

bool StmWriteResult::serialize(Serializer& s)
{
	return s >> Member< uint32_t >(L"result", m_result);
}

	}
}
