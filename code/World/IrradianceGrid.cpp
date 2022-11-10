/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "World/IrradianceGrid.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGrid", IrradianceGrid, Object)

IrradianceGrid::IrradianceGrid(
	gridSize_t size,
	const Aabb3& boundingBox,
	render::Buffer* buffer
)
:   m_boundingBox(boundingBox)
,   m_buffer(buffer)
{
	std::memcpy(m_size, size, sizeof(gridSize_t));
}

}
