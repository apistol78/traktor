/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionValuePool_H
#define traktor_flash_ActionValuePool_H

#include "Core/Misc/AutoPtr.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionValue;

/*! \brief ActionValue pool.
 * \ingroup Flash
 */
class T_DLLCLASS ActionValuePool
{
public:
	enum { Capacity = 131070 };

	class Scope
	{
	public:
		Scope(ActionValuePool& pool)
		:	m_pool(pool)
		,	m_offset(0)
		{
			m_offset = m_pool.offset();
		}

		~Scope()
		{
			m_pool.rewind(m_offset);
		}

	private:
		ActionValuePool& m_pool;
		uint32_t m_offset;
	};

	ActionValuePool();

	/*! \brief Allocate N values from pool.
	 *
	 * \param count Number of values.
	 * \return Pointer to first value.
	 */
	ActionValue* alloc(uint32_t count)
	{
		T_FATAL_ASSERT_M (m_next + count <= Capacity, L"Out of memory in pool");
		ActionValue* value = &m_top[m_next];
		m_next += count;
		return value;
	}

	/*! \brief Number of values allocated from pool.
	 *
	 * \return Offset from first pool value.
	 */
	uint32_t offset() const
	{
		return m_next;
	}

	/*! \brief Rewind, or release, N values back to pool.
	 *
	 * \param offset Offset from first pool value.
	 */
	void rewind(uint32_t offset)
	{
		T_ASSERT (offset <= m_next);
		for (uint32_t i = offset; i < m_next; ++i)
			m_top[i].clear();
		m_next = offset;
	}

	/*! \brief Rewind all values back to pool. */
	void flush()
	{
		rewind(0);
	}

private:
	AutoArrayPtr< ActionValue > m_top;
	uint32_t m_next;
};

	}
}

#endif	// traktor_flash_ActionValuePool_H
