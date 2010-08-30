#ifndef traktor_Thread_H
#define traktor_Thread_H

#include <string>
#include "Core/Ref.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Functor;

/*! \brief OS thread class.
 * \ingroup Core
 */
class T_DLLCLASS Thread : public IWaitable
{
public:
	enum Priority
	{
		Lowest = -2,
		Below = -1,
		Normal = 0,
		Above = 1,
		Highest = 2
	};

	bool start(Priority priority = Normal);

	bool stop(int timeout = -1);

	bool pause();

	bool resume();

	void sleep(int duration);

	void yield();

	bool current() const;

	bool stopped() const;

	bool finished() const;

	virtual bool wait(int32_t timeout = -1);

private:
	friend class ThreadManager;

	void* m_handle;
	uint32_t m_id;
	bool m_stopped;
	Ref< Functor > m_functor;
	std::string m_name;
	int m_hardwareCore;

	Thread(Functor* functor, const std::wstring& name, int hardwareCore);
	
	~Thread();
};

}

#endif	// traktor_Thread_H
