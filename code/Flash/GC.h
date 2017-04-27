/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_GC_H
#define traktor_flash_GC_H

#include "Core/Containers/IntrusiveList.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

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
class GC : public ISingleton
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

#endif	// traktor_flash_GC_H
