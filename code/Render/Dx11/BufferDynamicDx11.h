/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/BufferDx11.h"
#include "Render/Dx11/BufferViewDx11.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

class BufferDynamicDx11 : public BufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< BufferDynamicDx11 > create(
		ContextDx11* context,
		uint32_t usage,
		uint32_t elementCount,
		uint32_t elementSize
	);

	virtual void destroy() override;

	virtual void* lock() override;

	virtual void unlock() override;

	virtual const IBufferView* getBufferView() const override;

private:
	ComRef< ID3D11Buffer > m_d3dBuffer;
	ComRef< ID3D11ShaderResourceView > m_d3dBufferResourceView;
	ComRef< ID3D11UnorderedAccessView > m_d3dBufferUnorderedView;
	BufferViewDx11 m_bufferView;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_data;

	explicit BufferDynamicDx11(ContextDx11* context, uint32_t elementCount, uint32_t elementSize);
};
	
	}
}
