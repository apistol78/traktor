#include "Render/Dx9/Platform.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/VertexDeclCache.h"
#include "Render/Dx9/ContextDx9.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx9", VertexBufferDx9, VertexBuffer)

VertexBufferDx9* VertexBufferDx9::ms_activeVertexBuffer = 0;

VertexBufferDx9::VertexBufferDx9(UnmanagedListener* unmanagedListener, ContextDx9* context, uint32_t bufferSize, VertexDeclCache* vertexDeclCache)
:	VertexBuffer(bufferSize)
,	Unmanaged(unmanagedListener)
,	m_context(context)
,	m_vertexDeclCache(vertexDeclCache)
,	m_d3dVertexStride(0)
,	m_locked(false)
,	m_dynamic(false)
{
	Unmanaged::addToListener();
}

VertexBufferDx9::~VertexBufferDx9()
{
	destroy();
}

bool VertexBufferDx9::create(IDirect3DDevice9* d3dDevice, const std::vector< VertexElement >& vertexElements, bool dynamic)
{
	HRESULT hr;
	DWORD usage;

	if (!m_vertexDeclCache->createDeclaration(vertexElements, m_d3dVertexDeclaration, m_d3dVertexStride))
		return false;

	T_ASSERT ((getBufferSize() % m_d3dVertexStride) == 0);

#if defined(_XBOX) || defined(T_USE_XDK)
	usage = D3DUSAGE_WRITEONLY;
#else
	usage = D3DUSAGE_WRITEONLY | (dynamic ? D3DUSAGE_DYNAMIC : 0);
#endif

	hr = d3dDevice->CreateVertexBuffer(
		getBufferSize(),
		usage,
		0,
		dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&m_d3dVertexBuffer.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create vertex buffer, HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	m_d3dDevice = d3dDevice;
	m_dynamic = dynamic;

	return true;
}

void VertexBufferDx9::activate(IDirect3DDevice9* d3dDevice, VertexBufferDx9* vertexBuffer)
{
	T_ASSERT (!vertexBuffer->m_locked);

	if (ms_activeVertexBuffer == vertexBuffer)
		return;

	vertexBuffer->m_vertexDeclCache->setDeclaration(vertexBuffer->m_d3dVertexDeclaration);

	d3dDevice->SetStreamSource(
		0,
		vertexBuffer->m_d3dVertexBuffer,
		0,
		vertexBuffer->m_d3dVertexStride
	);

	ms_activeVertexBuffer = vertexBuffer;
}

void VertexBufferDx9::destroy()
{
	T_ASSERT (!m_locked);
#if !defined(_XBOX) && !defined(T_USE_XDK)
	if (m_context)
	{
		m_context->releaseComRef(m_d3dDevice);
		m_context->releaseComRef(m_d3dVertexDeclaration);
		m_context->releaseComRef(m_d3dVertexBuffer);
	}
#endif
	Unmanaged::removeFromListener();
}

void* VertexBufferDx9::lock()
{
	if (m_locked)
		return 0;

	if (ms_activeVertexBuffer == this)
	{
		m_d3dDevice->SetStreamSource(0, 0, 0, 0);
		ms_activeVertexBuffer = 0;
	}

	VOID* ptr;

	if (!m_dynamic)
	{
		if (FAILED(m_d3dVertexBuffer->Lock(0, getBufferSize(), &ptr, 0)))
			return 0;
	}
	else
	{
		DWORD flags;
#if defined(_XBOX) || defined(T_USE_XDK)
		flags = D3DLOCK_NOOVERWRITE;
#else
		flags = D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE;
#endif
		if (FAILED(m_d3dVertexBuffer->Lock(0, getBufferSize(), &ptr, flags)))
			return 0;
	}

	m_locked = true;
	return ptr;
}

void* VertexBufferDx9::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	if (m_locked)
		return 0;

	if (ms_activeVertexBuffer == this)
	{
		m_d3dDevice->SetStreamSource(0, 0, 0, 0);
		ms_activeVertexBuffer = 0;
	}

	UINT offset = vertexOffset * m_d3dVertexStride;
	UINT size = vertexCount * m_d3dVertexStride;
	VOID* ptr;

	if (!m_dynamic)
	{
		if (FAILED(m_d3dVertexBuffer->Lock(offset, size, &ptr, 0)))
			return 0;
	}
	else
	{
		DWORD flags;
#if defined(_XBOX) || defined(T_USE_XDK)
		flags = D3DLOCK_NOOVERWRITE;
#else
		flags = D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE;
#endif
		if (FAILED(m_d3dVertexBuffer->Lock(offset, size, &ptr, flags)))
			return 0;
	}

	m_locked = true;
	return ptr;
}

void VertexBufferDx9::unlock()
{
	if (m_locked)
	{
		m_d3dVertexBuffer->Unlock();
		m_locked = false;
	}
}

void VertexBufferDx9::forceDirty()
{
	ms_activeVertexBuffer = 0;
}

HRESULT VertexBufferDx9::lostDevice()
{
	ms_activeVertexBuffer = 0;

	if (!m_dynamic)
		return S_OK;

	unlock();

	m_d3dDevice = 0;
	m_d3dVertexBuffer = 0;

	return S_OK;
}

HRESULT VertexBufferDx9::resetDevice(IDirect3DDevice9* d3dDevice)
{
	if (!m_dynamic)
		return S_OK;

	m_d3dDevice = d3dDevice;

#if defined(_XBOX) || defined(T_USE_XDK)
	DWORD usage = D3DUSAGE_WRITEONLY;
#else
	DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
#endif

	HRESULT hr = m_d3dDevice->CreateVertexBuffer(
		getBufferSize(),
		usage,
		0,
		D3DPOOL_DEFAULT,
		&m_d3dVertexBuffer.getAssign(),
		NULL
	);
	
	return hr;
}

	}
}
