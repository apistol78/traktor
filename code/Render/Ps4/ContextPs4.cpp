#include "Core/Thread/Acquire.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/MemoryHeapObjectPs4.h"
#include "Render/Ps4/MemoryHeapPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextPs4", ContextPs4, Object)

ContextPs4::ContextPs4(
	MemoryHeapPs4* heapOnion,
	MemoryHeapPs4* heapGarlic
)
:	m_heapOnion(heapOnion)
,	m_heapGarlic(heapGarlic)
{
}

bool ContextPs4::create()
{
	return true;
}

void ContextPs4::destroy()
{

}

void ContextPs4::deleteResource(DeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextPs4::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (AlignedVector< DeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

	}
}
