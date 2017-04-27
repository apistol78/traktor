/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ContextPs4_H
#define traktor_render_ContextPs4_H

#include <gnmx.h>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace render
	{

class MemoryHeapObjectPs4;
class MemoryHeapPs4;

/*! \brief GNM context.
 * \ingroup GNM
 */
class ContextPs4 : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Garbage delete callback.
	 *
	 * Context keeps a queue for deletion callbacks
	 * to ensure resources are deleted properly (not used, same thread etc).
	 */
	struct DeleteCallback
	{
		virtual ~DeleteCallback() {}

		virtual void deleteResource() = 0;
	};

	ContextPs4(
		MemoryHeapPs4* heapOnion,
		MemoryHeapPs4* heapGarlic
	);

	bool create();

	void destroy();

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

	Semaphore& getLock() { return m_lock; }

	MemoryHeapPs4* getHeapOnion() { return m_heapOnion; }

	MemoryHeapPs4* getHeapGarlic() { return m_heapGarlic; }

private:
	Semaphore m_lock;
	Ref< MemoryHeapPs4 > m_heapOnion;
	Ref< MemoryHeapPs4 > m_heapGarlic;
	AlignedVector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextPs4_H
