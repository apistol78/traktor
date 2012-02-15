#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextDx11", ContextDx11, Object)

ContextDx11::ContextDx11(
	ID3D11Device* d3dDevice,
	ID3D11DeviceContext* d3dDeviceContext,
	IDXGIFactory1* dxgiFactory,
	IDXGIOutput* dxgiOutput
)
:	m_d3dDevice(d3dDevice)
,	m_d3dDeviceContext(d3dDeviceContext)
,	m_dxgiFactory(dxgiFactory)
,	m_dxgiOutput(dxgiOutput)
{
}

void ContextDx11::deleteResource(DeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextDx11::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::vector< DeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

ContextDx11::ReleaseComObjectCallback::ReleaseComObjectCallback(IUnknown* unk)
:	m_unk(unk)
{
}

void ContextDx11::ReleaseComObjectCallback::deleteResource()
{
	m_unk.release();
	delete this;
}

ContextDx11::ReleaseComObjectArrayCallback::ReleaseComObjectArrayCallback(IUnknown** unks, size_t count)
:	m_unks(unks, count)
{
}

void ContextDx11::ReleaseComObjectArrayCallback::deleteResource()
{
	m_unks.resize(0);
	delete this;
}

	}
}
