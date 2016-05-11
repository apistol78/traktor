#ifndef traktor_flash_GC_H
#define traktor_flash_GC_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

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

class Collectable;

/*! \brief Cyclic object garbage collector.
 * \ingroup Flash
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

	uint32_t getCandidateCount() const;

	void collectCycles(bool full);

protected:
	virtual void destroy();

private:
	Semaphore m_lock;
	AlignedVector< Collectable* > m_candidates;

	virtual ~GC();
};

	}
}

#endif	// traktor_flash_GC_H
