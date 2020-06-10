#pragma once

#include "Core/Containers/IntrusiveList.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Collectable;

/*! Cyclic object garbage collector.
 * \ingroup Spark
 *
 * This collaborate with Collectable in order
 * to track reference cycles introduced by runtime.
 * It uses a simple tracing algorithm in order
 * to find isolated reference cycles.
 */
class T_DLLCLASS GC : public ISingleton
{
public:
	static GC& getInstance();

	void addCandidate(Collectable* object);

	void removeCandidate(Collectable* object);

	void collectCycles(bool full);

protected:
	virtual void destroy();

private:
	Semaphore m_lock;
	IntrusiveList< Collectable > m_candidates;

	virtual ~GC();
};

	}
}

