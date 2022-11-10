/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Render/Dx11/VertexLayoutDx11.h"
#include "Render/Dx11/TypesDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexLayoutDx11", VertexLayoutDx11, IVertexLayout)
	
Ref< VertexLayoutDx11 > VertexLayoutDx11::create(const AlignedVector< VertexElement >& vertexElements)
{
	Ref< VertexLayoutDx11 > layout = new VertexLayoutDx11();

	layout->m_d3dInputElements.resize(vertexElements.size());
	for (uint32_t i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT((int)vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT((int)vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		std::memset(&layout->m_d3dInputElements[i], 0, sizeof(D3D11_INPUT_ELEMENT_DESC));
		layout->m_d3dInputElements[i].SemanticName = c_dxgiInputSemantic[(int)vertexElements[i].getDataUsage()];
		layout->m_d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		layout->m_d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		layout->m_d3dInputElements[i].InputSlot = 0;
		layout->m_d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		layout->m_d3dInputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layout->m_d3dInputElements[i].InstanceDataStepRate = 0;
	}

	Murmur3 hash;
	hash.begin();
	hash.feed(&layout->m_d3dInputElements[0], layout->m_d3dInputElements.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
	hash.end();

	layout->m_d3dInputElementsHash = hash.get();
	layout->m_d3dStride = getVertexSize(vertexElements);

	return layout;
}

	}
}