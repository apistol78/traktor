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
#include "Core/Containers/AlignedVector.h"
#include "Render/IVertexLayout.h"
#include "Render/VertexElement.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class VertexLayoutDx11 : public IVertexLayout
{
	T_RTTI_CLASS;

public:
	static Ref< VertexLayoutDx11 > create(const AlignedVector< VertexElement >& vertexElements);

	const AlignedVector< D3D11_INPUT_ELEMENT_DESC >& getD3D11InputElements() const { return m_d3dInputElements; }

	uint32_t getD3D11InputElementsHash() const { return m_d3dInputElementsHash; }

	UINT getD3D11Stride() const { return m_d3dStride; }

private:
	AlignedVector< D3D11_INPUT_ELEMENT_DESC > m_d3dInputElements;
	uint32_t m_d3dInputElementsHash;
	UINT m_d3dStride;
};

	}
}