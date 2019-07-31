#include "Spark/Action/ActionValue.h"
#include "Spark/Action/ActionValuePool.h"

namespace traktor
{
	namespace spark
	{

ActionValuePool::ActionValuePool()
:	m_next(0)
{
	m_top.reset(new ActionValue [Capacity]);
	T_FATAL_ASSERT_M (m_top.ptr(), L"Out of memory");
}

	}
}
