/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/BufferStaticDx11.h"
#include "Render/Dx11/ContextDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferStaticDx11", BufferStaticDx11, BufferDx11)

Ref< BufferStaticDx11 > BufferStaticDx11::create(
	ContextDx11* context,
	uint32_t usage,
	uint32_t elementCount,
	uint32_t elementSize
)
{
	D3D11_BUFFER_DESC dbd = {};
	HRESULT hr;

	Ref< BufferStaticDx11 > buffer = new BufferStaticDx11(context, elementCount, elementSize);

	dbd.ByteWidth = elementCount * elementSize;
	dbd.Usage = D3D11_USAGE_DEFAULT;

	dbd.BindFlags = 0;
	if ((usage & BuVertex) != 0)
		dbd.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
	if ((usage & BuIndex) != 0)
		dbd.BindFlags |= D3D11_BIND_INDEX_BUFFER;
	if ((usage & BuStructured) != 0)
	{
		dbd.BindFlags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		dbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		dbd.StructureByteStride = elementSize;
	}

	dbd.CPUAccessFlags = 0;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, nullptr, &buffer->m_d3dBuffer.getAssign());
	if (FAILED(hr))
		return nullptr;

	if ((usage & BuStructured) != 0)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {};
		dsrvd.Format = DXGI_FORMAT_UNKNOWN;
		dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		dsrvd.Buffer.ElementWidth = elementCount;
		hr = context->getD3DDevice()->CreateShaderResourceView(buffer->m_d3dBuffer, &dsrvd, &buffer->m_d3dBufferResourceView.getAssign());
		if (FAILED(hr))
			return nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC duav = {};
		duav.Format = DXGI_FORMAT_UNKNOWN;
		duav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		duav.Buffer.NumElements = elementCount;
		hr = context->getD3DDevice()->CreateUnorderedAccessView(buffer->m_d3dBuffer, &duav, &buffer->m_d3dBufferUnorderedView.getAssign());
		if (FAILED(hr))
			return nullptr;
	}

	buffer->m_bufferView = BufferViewDx11(buffer->m_d3dBuffer, buffer->m_d3dBufferResourceView);

	return buffer;
}

void BufferStaticDx11::destroy()
{
	if (m_context)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
		m_context->releaseComRef(m_d3dBuffer);
		m_context = nullptr;
	}
}

void* BufferStaticDx11::lock()
{
	T_ASSERT(!m_data.ptr());
	m_data.reset(new uint8_t [getBufferSize()]);
	return m_data.ptr();
}

void BufferStaticDx11::unlock()
{
	T_ASSERT(m_data.ptr());
	D3D11_BOX d3db;

	d3db.left = 0;
	d3db.right = getBufferSize();
	d3db.top = 0;
	d3db.bottom = 1;
	d3db.front = 0;
	d3db.back = 1;

	m_context->getD3DDeviceContext()->UpdateSubresource(
		m_d3dBuffer,
		0,
		&d3db,
		m_data.c_ptr(),
		0,
		0
	);

	m_data.release();
}

const IBufferView* BufferStaticDx11::getBufferView() const
{
	return &m_bufferView;
}

BufferStaticDx11::BufferStaticDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize)
:	BufferDx11(context, elementCount, elementSize)
{
}

	}
}
