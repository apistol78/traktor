#include "Core/Log/Log.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/VertexBufferDynamicDx9.h"
#include "Render/Dx9/VertexDeclCache.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicDx9", VertexBufferDynamicDx9, VertexBufferDx9)

VertexBufferDynamicDx9::VertexBufferDynamicDx9(ResourceManagerDx9* resourceManager, uint32_t bufferSize, VertexDeclCache* vertexDeclCache)
:	VertexBufferDx9(bufferSize)
,	m_resourceManager(resourceManager)
,	m_vertexDeclCache(vertexDeclCache)
,	m_d3dVertexStride(0)
,	m_dirty(false)
,	m_locked(false)
{
	m_resourceManager->add(this);
}

VertexBufferDynamicDx9::~VertexBufferDynamicDx9()
{
	destroy();
}

bool VertexBufferDynamicDx9::create(IDirect3DDevice9* d3dDevice, const std::vector< VertexElement >& vertexElements)
{
	HRESULT hr;
	DWORD usage;

	if (!m_vertexDeclCache->createDeclaration(vertexElements, m_d3dVertexDeclaration, m_d3dVertexStride))
		return false;

	T_ASSERT ((getBufferSize() % m_d3dVertexStride) == 0);

#if defined(_XBOX) || defined(T_USE_XDK)
	usage = D3DUSAGE_WRITEONLY;
#else
	usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
#endif

	d3dDevice->EvictManagedResources();

	hr = d3dDevice->CreateVertexBuffer(
		getBufferSize(),
		usage,
		0,
		D3DPOOL_DEFAULT,
		&m_d3dVertexBuffer.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create vertex buffer, HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	m_buffer.resize(getBufferSize(), 0);

	m_d3dDevice = d3dDevice;

	return true;
}

bool VertexBufferDynamicDx9::activate(IDirect3DDevice9* d3dDevice)
{
	T_ASSERT (!m_locked);

	// Upload vertex buffer data if it's been modified.
	if (m_dirty)
	{
		VOID* ptr;
		DWORD flags;
#if defined(_XBOX) || defined(T_USE_XDK)
		flags = D3DLOCK_NOOVERWRITE;
#else
		flags = D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE;
#endif
		if (FAILED(m_d3dVertexBuffer->Lock(0, getBufferSize(), &ptr, flags)))
			return 0;

		std::memcpy(ptr, &m_buffer[0], getBufferSize());

		m_d3dVertexBuffer->Unlock();
		m_dirty = false;
	}

	m_vertexDeclCache->setDeclaration(m_d3dVertexDeclaration);

	d3dDevice->SetStreamSource(
		0,
		m_d3dVertexBuffer,
		0,
		m_d3dVertexStride
	);

	return true;
}

void VertexBufferDynamicDx9::destroy()
{
	T_ASSERT (!m_locked);
	m_d3dDevice.release();
	m_d3dVertexBuffer.release();
	m_resourceManager->remove(this);
}

void* VertexBufferDynamicDx9::lock()
{
	if (m_locked)
		return 0;

	m_locked = true;
	return &m_buffer[0];
}

void* VertexBufferDynamicDx9::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	if (m_locked)
		return 0;

	m_locked = true;
	return &m_buffer[vertexOffset * m_d3dVertexStride];
}

void VertexBufferDynamicDx9::unlock()
{
	if (m_locked)
	{
		m_dirty = true;
		m_locked = false;
	}
	setContentValid(true);
}

HRESULT VertexBufferDynamicDx9::lostDevice()
{
	m_d3dDevice.release();
	m_d3dVertexBuffer.release();
	setContentValid(false);
	return S_OK;
}

HRESULT VertexBufferDynamicDx9::resetDevice(IDirect3DDevice9* d3dDevice)
{
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
