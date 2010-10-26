#ifndef traktor_flash_ActionObjectCyclic_H
#define traktor_flash_ActionObjectCyclic_H

#include <list>
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace flash
	{

class ActionObject;

/*! \brief ActionScript object cyclic collector.
 * \ingroup Flash
 *
 * This collaborate with ActionObject in order
 * to track reference cycles introduced by AS.
 * It uses a simple tracing algorithm in order
 * to find isolated reference cycles.
 */
class ActionObjectCyclic : public ISingleton
{
public:
	static ActionObjectCyclic& getInstance();

	void addCandidate(ActionObject* object);

	void removeCandidate(ActionObject* object);

	void collectCycles(bool full);

protected:
	virtual void destroy();

private:
	Semaphore m_lock;
	std::list< ActionObject* > m_candidates;

	virtual ~ActionObjectCyclic();
};

	}
}

#endif	// traktor_flash_ActionObjectCyclic_H
