#ifndef traktor_flash_ActionValueStack_H
#define traktor_flash_ActionValueStack_H

#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript value stack.
 * \ingroup Flash
 *
 * Only intended to be used in implementation.
 */
class ActionValueStack
{
public:
	ActionValueStack(ActionValuePool& pool)
	:	m_stack(pool.alloc(MaxStackDepth))
	,	m_index(0)
	{
	}

	/*! \brief Push value onto stack.
	 *
	 * \param value Value to push.
	 */
	void push(const ActionValue& value)
	{
		T_ASSERT (m_index < MaxStackDepth);
		m_stack[m_index++] = value;
	}

	/*! \brief Pop value of stack.
	 *
	 * \return Value.
	 */
	const ActionValue& pop()
	{
		return m_index > 0 ? m_stack[--m_index] : ms_undefined;
	}

	/*! \brief Peek at value on top of stack.
	 *
	 * \param offset Offset from top of stack.
	 * \return Value.
	 */
	ActionValue& top(int32_t offset = 0)
	{
		int32_t index = m_index + offset - 1;
		return (index >= 0 && index < MaxStackDepth) ? m_stack[index] : ms_undefined;
	}

	/*! \brief Peek at value on top of stack.
	 *
	 * \param offset Offset from top of stack.
	 * \return Value.
	 */
	const ActionValue& top(int32_t offset = 0) const
	{
		int32_t index = m_index + offset - 1;
		return (index >= 0 && index < MaxStackDepth) ? m_stack[index] : ms_undefined;
	}

	/*! \brief Stack empty.
	 *
	 * \return True if stack is empty.
	 */
	bool empty() const
	{
		return m_index <= 0;
	}

	/*! \brief Ensure stack contains given number of values.
	 *
	 * \param ensureCount Number of values.
	 */
	void ensure(int32_t ensureCount)
	{
		T_ASSERT (ensureCount < MaxStackDepth);
		while (m_index < ensureCount)
			m_stack[m_index++] = ms_undefined;
	}

	/*! \brief Drop values from stack.
	 *
	 * \param dropCount Number of values to drop.
	 */
	void drop(int32_t dropCount)
	{
		if ((m_index -= dropCount) < 0)
			m_index = 0;
	}

	/*! \brief Stack depth.
	 *
	 * \return Depth of stack.
	 */
	int32_t depth() const
	{
		return m_index;
	}

private:
	enum { MaxStackDepth = 256 };

	static ActionValue ms_undefined;
	ActionValue* m_stack;
	int32_t m_index;
};

	}
}

#endif	// traktor_flash_ActionValueStack_H
