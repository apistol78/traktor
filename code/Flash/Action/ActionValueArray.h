/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionValueArray_H
#define traktor_flash_ActionValueArray_H

#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript immutable value array.
 * \ingroup Flash
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
		T_ASSERT (index < m_size);
		return m_values[index];
	}
	
	const ActionValue& operator [] (uint32_t index) const
	{
		T_ASSERT (index < m_size);
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

#endif	// traktor_flash_ActionValueArray_H
