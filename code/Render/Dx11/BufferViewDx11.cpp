/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Dx11/BufferViewDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferViewDx11", BufferViewDx11, IBufferView)

BufferViewDx11::BufferViewDx11(ID3D11Buffer* d3dBuffer, ID3D11ShaderResourceView* d3dBufferResourceView)
:	m_d3dBuffer(d3dBuffer)
,	m_d3dBufferResourceView(d3dBufferResourceView)
{
}

	}
}