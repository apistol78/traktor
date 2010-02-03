#ifndef traktor_flash_ActionValueStack_H
#define traktor_flash_ActionValueStack_H

#include <vector>
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

/*! \brief ActionScript value stack.
 * \ingroup Flash
 */
class T_DLLCLASS ActionValueStack : public Object
{
	T_RTTI_CLASS;

public:
	ActionValueStack()
	:	m_index(0)
	{
	}

	/*! \brief Push value onto stack.
	 *
	 * \param value Value to push.
	 */
	inline void push(const ActionValue& value)
	{
		T_ASSERT (m_index < MaxStackDepth);
		m_stack[m_index++] = value;
	}

	/*! \brief Pop value of stack.
	 *
	 * \return Value.
	 */
	inline ActionValue pop()
	{
		T_ASSERT (m_index > 0);
		return m_stack[--m_index];
	}

	/*! \brief Peek at value on top of stack.
	 *
	 * \param offset Offset from top of stack.
	 * \return Value.
	 */
	inline ActionValue& top(int32_t offset = 0)
	{
		return m_stack[m_index + offset - 1];
	}

	/*! \brief Stack empty.
	 *
	 * \return True if stack is empty.
	 */
	inline bool empty() const
	{
		return m_index <= 0;
	}

	/*! \brief Ensure stack contains given number of values.
	 *
	 * \param ensureCount Number of values.
	 */
	inline void ensure(int32_t ensureCount)
	{
		while (m_index < ensureCount)
			m_stack[m_index++] = ActionValue();
	}

	/*! \brief Drop values from stack.
	 *
	 * \param dropCount Number of values to drop.
	 */
	inline void drop(int32_t dropCount)
	{
		m_index -= dropCount;
		m_index = std::max(m_index, 0);
	}

	/*! \brief Stack depth.
	 *
	 * \return Depth of stack.
	 */
	inline int32_t depth() const
	{
		return m_index;
	}

private:
	enum { MaxStackDepth = 64 };
	ActionValue m_stack[MaxStackDepth];
	int32_t m_index;
};

	}
}

#endif	// traktor_flash_ActionValueStack_H
