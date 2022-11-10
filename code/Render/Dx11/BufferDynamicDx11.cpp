/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <emmintrin.h>
#include <xmmintrin.h>
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/BufferDynamicDx11.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void copyBuffer(uint8_t* dst, const uint8_t* src, uint32_t size)
{
	uint32_t i = 0;
	for (; i + 128 <= size; i += 128)
	{
		__m128i d0 = _mm_load_si128((__m128i*)&src[i + 0 * 16]);
		__m128i d1 = _mm_load_si128((__m128i*)&src[i + 1 * 16]);
		__m128i d2 = _mm_load_si128((__m128i*)&src[i + 2 * 16]);
		__m128i d3 = _mm_load_si128((__m128i*)&src[i + 3 * 16]);
		__m128i d4 = _mm_load_si128((__m128i*)&src[i + 4 * 16]);
		__m128i d5 = _mm_load_si128((__m128i*)&src[i + 5 * 16]);
		__m128i d6 = _mm_load_si128((__m128i*)&src[i + 6 * 16]);
		__m128i d7 = _mm_load_si128((__m128i*)&src[i + 7 * 16]);
		_mm_stream_si128((__m128i*)&dst[i + 0 * 16], d0);
		_mm_stream_si128((__m128i*)&dst[i + 1 * 16], d1);
		_mm_stream_si128((__m128i*)&dst[i + 2 * 16], d2);
		_mm_stream_si128((__m128i*)&dst[i + 3 * 16], d3);
		_mm_stream_si128((__m128i*)&dst[i + 4 * 16], d4);
		_mm_stream_si128((__m128i*)&dst[i + 5 * 16], d5);
		_mm_stream_si128((__m128i*)&dst[i + 6 * 16], d6);
		_mm_stream_si128((__m128i*)&dst[i + 7 * 16], d7);
	}
	for (; i + 16 <= size; i += 16)
	{
		__m128i d = _mm_load_si128((__m128i*)&src[i]);
		_mm_stream_si128((__m128i *)&dst[i], d);
	}
	for (; i + 4 <= size; i += 4)
	{
		*(uint32_t*)&dst[i] = *(const uint32_t*)&src[i];
	}
	for (; i < size; i++)
	{
		dst[i] = src[i];
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferDynamicDx11", BufferDynamicDx11, BufferDx11)

Ref< BufferDynamicDx11 > BufferDynamicDx11::create(
	ContextDx11* context,
	uint32_t usage,
	uint32_t elementCount,
	uint32_t elementSize
)
{
	D3D11_BUFFER_DESC dbd = {};
	HRESULT hr;

	Ref< BufferDynamicDx11 > buffer = new BufferDynamicDx11(context, elementCount, elementSize);

	dbd.ByteWidth = elementCount * elementSize;
	dbd.Usage = D3D11_USAGE_DYNAMIC;

	dbd.BindFlags = 0;
	if ((usage & BuVertex) != 0)
		dbd.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
	if ((usage & BuIndex) != 0)
		dbd.BindFlags |= D3D11_BIND_INDEX_BUFFER;
	if ((usage & BuStructured) != 0)
	{
		dbd.BindFlags |= D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_UNORDERED_ACCESS;
		dbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		dbd.StructureByteStride = elementSize;
	}

	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, NULL, &buffer->m_d3dBuffer.getAssign());
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

		//D3D11_UNORDERED_ACCESS_VIEW_DESC duav = {};
		//duav.Format = DXGI_FORMAT_UNKNOWN;
		//duav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		//duav.Buffer.NumElements = elementCount;
		//hr = context->getD3DDevice()->CreateUnorderedAccessView(buffer->m_d3dBuffer, &duav, &buffer->m_d3dBufferUnorderedView.getAssign());
		//if (FAILED(hr))
		//	return nullptr;
	}

	buffer->m_bufferView = BufferViewDx11(buffer->m_d3dBuffer, buffer->m_d3dBufferResourceView);
	buffer->m_data.reset((uint8_t*)Alloc::acquireAlign(elementCount * elementSize, 16, T_FILE_LINE));

	return buffer;
}

void BufferDynamicDx11::destroy()
{
	if (m_context)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
		m_context->releaseComRef(m_d3dBuffer);
		m_context = nullptr;
	}
}

void* BufferDynamicDx11::lock()
{
	return m_data.ptr();
}

void BufferDynamicDx11::unlock()
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	// See GCN Performance Tip 29: http://developer.amd.com/wordpress/media/2013/05/GCNPerformanceTweets.pdf
	hr = m_context->getD3DDeviceContext()->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return;

	copyBuffer((uint8_t*)dm.pData, m_data.c_ptr(), getBufferSize());

	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
}

const IBufferView* BufferDynamicDx11::getBufferView() const
{
	return &m_bufferView;
}

BufferDynamicDx11::BufferDynamicDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize)
:	BufferDx11(context, elementCount, elementSize)
{
}

	}
}
