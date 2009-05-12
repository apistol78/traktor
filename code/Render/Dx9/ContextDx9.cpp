#include "Render/Dx9/ContextDx9.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextDx9", ContextDx9, Object)

void ContextDx9::deleteResource(DeleteCallback* callback)
{
	Acquire< Semaphore > lock(m_deleteResourcesLock);
	m_deleteResources.push_back(callback);
}

void ContextDx9::deleteResources()
{
	Acquire< Semaphore > lock(m_deleteResourcesLock);
	for (std::vector< DeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

ContextDx9::ReleaseComObjectCallback::ReleaseComObjectCallback(IUnknown* unk)
:	m_unk(unk)
{
}

void ContextDx9::ReleaseComObjectCallback::deleteResource()
{
	m_unk.release();
	delete this;
}

ContextDx9::ReleaseComObjectArrayCallback::ReleaseComObjectArrayCallback(IUnknown** unks, size_t count)
:	m_unks(unks, count)
{
}

void ContextDx9::ReleaseComObjectArrayCallback::deleteResource()
{
	m_unks.resize(0);
	delete this;
}

	}
}
