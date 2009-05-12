#ifndef traktor_flash_ActionValueStack_H
#define traktor_flash_ActionValueStack_H

#include <vector>
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(ActionValueStack)

public:
	/*! \brief Push value onto stack.
	 *
	 * \param value Value to push.
	 */
	inline void push(const ActionValue& value)
	{
		m_stack.push_back(value);
	}

	/*! \brief Pop value of stack.
	 *
	 * \return Value.
	 */
	inline ActionValue pop()
	{
		ActionValue value = m_stack.back();
		m_stack.pop_back();
		return value;
	}

	/*! \brief Peek at value on top of stack.
	 *
	 * \param offset Offset from top of stack.
	 * \return Value.
	 */
	inline ActionValue& top(int offset = 0)
	{
		return m_stack[m_stack.size() + offset - 1];
	}

	/*! \brief Stack empty.
	 *
	 * \return True if stack is empty.
	 */
	inline bool empty() const
	{
		return m_stack.empty();
	}

	/*! \brief Ensure stack contains given number of values.
	 *
	 * \param ensureCount Number of values.
	 */
	inline void ensure(int ensureCount)
	{
		while (int(m_stack.size()) < ensureCount)
			m_stack.push_back(ActionValue());
	}

	/*! \brief Drop values from stack.
	 *
	 * \param dropCount Number of values to drop.
	 */
	inline void drop(int dropCount)
	{
		int newSize = int(m_stack.size() - dropCount);
		newSize = std::max(newSize, 0);
		m_stack.resize(newSize);
	}

	/*! \brief Stack depth.
	 *
	 * \return Depth of stack.
	 */
	inline uint32_t depth() const
	{
		return uint32_t(m_stack.size());
	}

private:
	std::vector< ActionValue > m_stack;
};

	}
}

#endif	// traktor_flash_ActionValueStack_H
