#pragma once

#include "Spark/Action/ActionValue.h"
#include "Spark/Action/ActionValuePool.h"

namespace traktor
{
	namespace spark
	{

/*! \brief ActionScript immutable value array.
 * \ingroup Spark
 *
 * Only intended to be used in implementation.
 */
class ActionValueArray
{
public:
	ActionValueArray()
	:	m_values(0)
	,	m_size(0)
	{
	}

	ActionValueArray(const ActionValueArray& arr)
	:	m_values(arr.m_values)
	,	m_size(arr.m_size)
	{
	}

	ActionValueArray(ActionValuePool& pool, uint32_t count)
	:	m_values(pool.alloc(count))
	,	m_size(count)
	{
	}

	bool empty() const
	{
		return m_size == 0;
	}

	uint32_t size() const
	{
		return m_size;
	}

	ActionValue& operator [] (uint32_t index)
	{
		T_ASSERT(index < m_size);
		return m_values[index];
	}

	const ActionValue& operator [] (uint32_t index) const
	{
		T_ASSERT(index < m_size);
		return m_values[index];
	}

	ActionValueArray& operator = (const ActionValueArray& arr)
	{
		m_values = arr.m_values;
		m_size = arr.m_size;
		return *this;
	}

private:
	ActionValue* m_values;
	uint32_t m_size;
};

	}
}

