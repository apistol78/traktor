/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/VertexBufferStaticDx9.h"
#include "Render/Dx9/VertexDeclCache.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticDx9", VertexBufferStaticDx9, VertexBufferDx9)

VertexBufferStaticDx9::VertexBufferStaticDx9(uint32_t bufferSize, VertexDeclCache* vertexDeclCache)
:	VertexBufferDx9(bufferSize)
,	m_vertexDeclCache(vertexDeclCache)
,	m_d3dVertexStride(0)
,	m_locked(false)
{
}

VertexBufferStaticDx9::~VertexBufferStaticDx9()
{
	destroy();
}

bool VertexBufferStaticDx9::create(IDirect3DDevice9* d3dDevice, const std::vector< VertexElement >& vertexElements)
{
	HRESULT hr;

	if (!m_vertexDeclCache->createDeclaration(vertexElements, m_d3dVertexDeclaration, m_d3dVertexStride))
		return false;

	T_ASSERT ((getBufferSize() % m_d3dVertexStride) == 0);

	hr = d3dDevice->CreateVertexBuffer(
		getBufferSize(),
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&m_d3dVertexBuffer.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create vertex buffer, HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	m_d3dDevice = d3dDevice;
	return true;
}

bool VertexBufferStaticDx9::activate(IDirect3DDevice9* d3dDevice)
{
	T_ASSERT (!m_locked);

	m_vertexDeclCache->setDeclaration(m_d3dVertexDeclaration);

	d3dDevice->SetStreamSource(
		0,
		m_d3dVertexBuffer,
		0,
		m_d3dVertexStride
	);

	return true;
}

void VertexBufferStaticDx9::destroy()
{
	T_ASSERT (!m_locked);
	m_d3dDevice.release();
	m_d3dVertexBuffer.release();
}

void* VertexBufferStaticDx9::lock()
{
	if (m_locked)
		return 0;

	VOID* ptr;
	if (FAILED(m_d3dVertexBuffer->Lock(0, 0, &ptr, 0)))
		return 0;

	m_locked = true;
	return ptr;
}

void* VertexBufferStaticDx9::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	if (m_locked)
		return 0;

	UINT offset = vertexOffset * m_d3dVertexStride;
	UINT size = vertexCount * m_d3dVertexStride;

	VOID* ptr;
	if (FAILED(m_d3dVertexBuffer->Lock(offset, size, &ptr, D3DLOCK_NOSYSLOCK)))
		return 0;

	m_locked = true;
	return ptr;
}

void VertexBufferStaticDx9::unlock()
{
	if (m_locked)
	{
		m_d3dVertexBuffer->Unlock();
		m_locked = false;
	}
	setContentValid(true);
}

	}
}
