#pragma once

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

/*! Job handle object.
 * \ingroup Core
 */
class T_DLLCLASS Job
:	public RefCountImpl< IRefCount >
,	public IWaitable
{
public:
	virtual bool wait(int32_t timeout = -1) override final;

	void stop();

	bool stopped() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	friend class JobQueue;

	Ref< Functor > m_functor;
	uint32_t m_finished;

	explicit Job(Functor* functor);
};

}

