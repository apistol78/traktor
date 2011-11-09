#if defined(_DEBUG)
#	include "Core/Thread/ThreadManager.h"
#endif
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_poolValueCount = 65535;

		}

ActionValuePool::ActionValuePool()
:	m_next(0)
#if defined(_DEBUG)
,	m_ownerThread(0)
#endif
{
#if defined(_DEBUG)
	m_ownerThread = ThreadManager::getInstance().getCurrentThread();
#endif
	m_top.reset(new ActionValue [c_poolValueCount]);
	T_FATAL_ASSERT_M (m_top.ptr(), L"Out of memory");
}

ActionValue* ActionValuePool::alloc(uint32_t count)
{
	T_ASSERT (m_ownerThread == ThreadManager::getInstance().getCurrentThread());
	T_FATAL_ASSERT_M (m_next + count <= c_poolValueCount, L"Out of memory in pool");

	ActionValue* value = &m_top[m_next];
	m_next += count;

	return value;
}

uint32_t ActionValuePool::offset() const
{
	T_ASSERT (m_ownerThread == ThreadManager::getInstance().getCurrentThread());
	return m_next;
}

void ActionValuePool::rewind(uint32_t offset)
{
	T_ASSERT (m_ownerThread == ThreadManager::getInstance().getCurrentThread());
	T_ASSERT (offset <= m_next);

	for (uint32_t i = offset; i < m_next; ++i)
		m_top[i] = ActionValue();

	m_next = offset;
}

void ActionValuePool::flush()
{
	T_ASSERT (m_ownerThread == ThreadManager::getInstance().getCurrentThread());
	rewind(0);
}

	}
}
