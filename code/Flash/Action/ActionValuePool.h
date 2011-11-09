#ifndef traktor_flash_ActionValuePool_H
#define traktor_flash_ActionValuePool_H

#include "Core/Misc/AutoPtr.h"

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

	ActionValue* alloc(uint32_t count);

	uint32_t offset() const;

	void rewind(uint32_t offset);

	void flush();

private:
	AutoArrayPtr< ActionValue > m_top;
	uint32_t m_next;
#if defined(_DEBUG)
	const class Thread* m_ownerThread;
#endif
};

	}
}

#endif	// traktor_flash_ActionValuePool_H
