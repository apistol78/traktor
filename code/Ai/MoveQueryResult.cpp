#include "Ai/MoveQueryResult.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.MoveQueryResult", MoveQueryResult, Result)

void MoveQueryResult::succeed(MoveQuery* moveQuery)
{
	m_moveQuery = moveQuery;
	Result::succeed();
}

MoveQuery* MoveQueryResult::get() const
{
	wait();
	return m_moveQuery;
}

	}
}
