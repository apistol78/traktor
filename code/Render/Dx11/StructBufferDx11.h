#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/ComRef.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Dx11/BufferViewDx11.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
	
class StructBufferDx11 : public StructBuffer
{
	T_RTTI_CLASS;

public:
	static Ref< StructBufferDx11 > create(
		ContextDx11* context,
		const AlignedVector< StructElement >& structElements,
		uint32_t bufferSize
	);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const;

	ID3D11ShaderResourceView* getD3D11ShaderResourceView() { return m_d3dBufferResourceView; }

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Buffer > m_d3dBuffer;
	ComRef< ID3D11ShaderResourceView > m_d3dBufferResourceView;
	ComRef< ID3D11UnorderedAccessView > m_d3dBufferUnorderedView;
	BufferViewDx11 m_bufferView;

	explicit StructBufferDx11(uint32_t bufferSize);
};

	}
}
