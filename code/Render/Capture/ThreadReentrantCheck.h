#ifndef traktor_render_ThreadReentrantCheck_H
#define traktor_render_ThreadReentrantCheck_H

#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace render
	{
	
/*! \brief
 * \ingroup Capture
 */
class ThreadReentrantCheck
{
public:
	ThreadReentrantCheck()
	:	m_lock(0)
	{
	}

	void wait()
	{
		T_FATAL_ASSERT_M (Atomic::exchange(m_lock, 1) == 0, L"Call not thread safe.");
	}

	void release()
	{
		Atomic::exchange(m_lock, 0);
	}

private:
	int32_t m_lock;
};
	
	}
}

#endif	// traktor_render_ThreadReentrantCheck_H

