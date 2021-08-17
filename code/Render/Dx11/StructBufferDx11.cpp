#include <cstring>
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/StructBufferDx11.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferDx11", StructBufferDx11, StructBuffer)

Ref< StructBufferDx11 > StructBufferDx11::create(
	ContextDx11* context,
	const AlignedVector< StructElement >& structElements,
	uint32_t bufferSize
)
{
	const bool cpuWritable = true;
	const bool gpuWritable = false;

	Ref< StructBufferDx11 > structBuffer = new StructBufferDx11(bufferSize);
	structBuffer->m_context = context;

	HRESULT hr;

	D3D11_BUFFER_DESC dbd = { 0 };
	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	dbd.StructureByteStride = getStructSize(structElements);

	if (!cpuWritable && !gpuWritable)
	{
		dbd.Usage = D3D11_USAGE_IMMUTABLE;
		dbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		dbd.CPUAccessFlags = 0;
	}
	else if (cpuWritable && !gpuWritable)
	{
		dbd.Usage = D3D11_USAGE_DYNAMIC;
		dbd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (!cpuWritable && gpuWritable)
	{
		dbd.Usage = D3D11_USAGE_DEFAULT;
		dbd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		dbd.CPUAccessFlags = 0;
	}
	else
		return nullptr;

	hr = context->getD3DDevice()->CreateBuffer(&dbd, nullptr, &structBuffer->m_d3dBuffer.getAssign());
	if (FAILED(hr))
		return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	std::memset(&dsrvd, 0, sizeof(dsrvd));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.ElementWidth = bufferSize / getStructSize(structElements);
	hr = context->getD3DDevice()->CreateShaderResourceView(structBuffer->m_d3dBuffer, &dsrvd, &structBuffer->m_d3dBufferResourceView.getAssign());
	if (FAILED(hr))
		return nullptr;

	if (gpuWritable)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC duav;
		std::memset(&duav, 0, sizeof(duav));
		duav.Format = DXGI_FORMAT_UNKNOWN;
		duav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		duav.Buffer.NumElements = bufferSize / getStructSize(structElements);
		hr = context->getD3DDevice()->CreateUnorderedAccessView(structBuffer->m_d3dBuffer, &duav, &structBuffer->m_d3dBufferUnorderedView.getAssign());
		if (FAILED(hr))
			return nullptr;
	}

	structBuffer->m_bufferView = BufferViewDx11(structBuffer->m_d3dBufferResourceView);

	return structBuffer;
}

void StructBufferDx11::destroy()
{
	if (m_context)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
		m_context->releaseComRef(m_d3dBufferUnorderedView);
		m_context->releaseComRef(m_d3dBufferResourceView);
		m_context->releaseComRef(m_d3dBuffer);
	}
}

void* StructBufferDx11::lock()
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	hr = m_context->getD3DDeviceContext()->Map(m_d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return nullptr;

	return dm.pData;
}

void StructBufferDx11::unlock()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	m_context->getD3DDeviceContext()->Unmap(m_d3dBuffer, 0);
}

const IBufferView* StructBufferDx11::getBufferView() const
{
	return &m_bufferView;
}

StructBufferDx11::StructBufferDx11(uint32_t bufferSize)
:	StructBuffer(bufferSize)
{
}

	}
}