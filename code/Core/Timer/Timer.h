#ifndef traktor_Timer_H
#define traktor_Timer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief High quality timer.
 * \ingroup Core
 */
class T_DLLCLASS Timer : public Object
{
	T_RTTI_CLASS;

public:
	Timer();

	/*! \brief Start timer. */
	void start();
	
	/*! \brief Pause timer. */
	void pause();
	
	/*! \brief Stop timer. */
	void stop();

	/*! \brief Is timer started? */
	bool started() const { return !m_paused; }
	
	/*! \brief Get number of seconds since timer started. */
	double getElapsedTime() const;
	
	/*! \brief Get number of seconds since last call to this method. */
	double getDeltaTime();
	
private:
	int64_t m_frequency;
	int64_t m_first;
	int64_t m_last;
	bool m_paused;
};
	
}

#endif	// traktor_Timer_H
