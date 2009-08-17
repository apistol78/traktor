#include "Render/Dx9/Platform.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/ContextDx9.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDx9", IndexBufferDx9, IndexBuffer)

IndexBufferDx9* IndexBufferDx9::ms_activeIndexBuffer = 0;

IndexBufferDx9::IndexBufferDx9(UnmanagedListener* unmanagedListener, ContextDx9* context, IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	Unmanaged(unmanagedListener)
,	m_context(context)
,	m_d3dFormat(D3DFMT_UNKNOWN)
,	m_dynamic(false)
{
	switch (indexType)
	{
	case ItUInt16:
		m_d3dFormat = D3DFMT_INDEX16;
		break;

	case ItUInt32:
		m_d3dFormat = D3DFMT_INDEX32;
		break;
	}
	Unmanaged::addToListener();
}

IndexBufferDx9::~IndexBufferDx9()
{
	destroy();
}

bool IndexBufferDx9::create(IDirect3DDevice9* d3dDevice, bool dynamic)
{
	HRESULT hr;
	DWORD usage;
	
#if defined(_XBOX) || defined(T_USE_XDK)
	usage = D3DUSAGE_WRITEONLY;
#else
	usage = D3DUSAGE_WRITEONLY | (dynamic ? D3DUSAGE_DYNAMIC : 0);
#endif

	hr = d3dDevice->CreateIndexBuffer(
		getBufferSize(),
		usage,
		m_d3dFormat,
		dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&m_d3dIndexBuffer.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create index buffer (" << hr << L")" << Endl;
		return false;
	}

	m_dynamic = dynamic;
	return true;
}

void IndexBufferDx9::activate(IDirect3DDevice9* d3dDevice)
{
	if (ms_activeIndexBuffer == this)
		return;

	if (this)
		d3dDevice->SetIndices(m_d3dIndexBuffer);
	else
		d3dDevice->SetIndices(NULL);

	ms_activeIndexBuffer = this;
}

void IndexBufferDx9::destroy()
{
#if !defined(_XBOX) && !defined(T_USE_XDK)
	if (m_context)
		m_context->releaseComRef(m_d3dIndexBuffer);
#endif
	Unmanaged::removeFromListener();
}

void* IndexBufferDx9::lock()
{
	VOID* ptr;
	
	if (FAILED(m_d3dIndexBuffer->Lock(0, getBufferSize(), &ptr, 0)))
		return 0;
	
	return ptr;
}

void IndexBufferDx9::unlock()
{
	m_d3dIndexBuffer->Unlock();
}

HRESULT IndexBufferDx9::lostDevice()
{
	ms_activeIndexBuffer = 0;

	if (!m_dynamic)
		return S_OK;

	m_d3dIndexBuffer = 0;
	return S_OK;
}

HRESULT IndexBufferDx9::resetDevice(IDirect3DDevice9* d3dDevice)
{
	if (!m_dynamic)
		return S_OK;

#if defined(_XBOX) || defined(T_USE_XDK)
	DWORD usage = D3DUSAGE_WRITEONLY;
#else
	DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
#endif

	HRESULT hr = d3dDevice->CreateIndexBuffer(
		getBufferSize(),
		usage,
		m_d3dFormat,
		D3DPOOL_DEFAULT,
		&m_d3dIndexBuffer.getAssign(),
		NULL
	);

	return hr;
}

	}
}
