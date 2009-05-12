#include "Render/Dx10/ContextDx10.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextDx10", ContextDx10, Object)

void ContextDx10::deleteResource(DeleteCallback* callback)
{
	Acquire< Semaphore > lock(m_deleteResourcesLock);
	m_deleteResources.push_back(callback);
}

void ContextDx10::deleteResources()
{
	Acquire< Semaphore > lock(m_deleteResourcesLock);
	for (std::vector< DeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

ContextDx10::ReleaseComObjectCallback::ReleaseComObjectCallback(IUnknown* unk)
:	m_unk(unk)
{
}

void ContextDx10::ReleaseComObjectCallback::deleteResource()
{
	m_unk.release();
	delete this;
}

ContextDx10::ReleaseComObjectArrayCallback::ReleaseComObjectArrayCallback(IUnknown** unks, size_t count)
:	m_unks(unks, count)
{
}

void ContextDx10::ReleaseComObjectArrayCallback::deleteResource()
{
	m_unks.resize(0);
	delete this;
}

	}
}
