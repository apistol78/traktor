/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/SH/SHCoeffs.h"
#include "World/IrradianceGrid.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGrid", IrradianceGrid, Object)

IrradianceGrid::IrradianceGrid(
	const gridSize_t size,
	const Aabb3& boundingBox,
	render::Buffer* buffer
)
:   m_boundingBox(boundingBox)
,   m_buffer(buffer)
{
	std::memcpy(m_size, size, sizeof(gridSize_t));
}

Ref< IrradianceGrid > IrradianceGrid::createSingle(render::IRenderSystem* renderSystem, const render::SHCoeffs& shCoeffs)
{
	Ref< render::Buffer > buffer = renderSystem->createBuffer(
		render::BuStructured,
		sizeof(IrradianceGridData),
		false
	);
	if (!buffer)
		return nullptr;

	IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
	T_FATAL_ASSERT(grid);

	auto& g = *grid;

	for (int32_t i = 0; i < 4; ++i)
	{
		const Vector4& shc = shCoeffs[i];
		g.shR0_3[i] = floatToHalf(shc.x());
		g.shG0_3[i] = floatToHalf(shc.y());
		g.shB0_3[i] = floatToHalf(shc.z());
	}
	for (int32_t i = 0; i < 4; ++i)
	{
		const Vector4& shc = shCoeffs[4 + i];
		g.shR4_7[i] = floatToHalf(shc.x());
		g.shG4_7[i] = floatToHalf(shc.y());
		g.shB4_7[i] = floatToHalf(shc.z());
	}

	const Vector4& shc = shCoeffs[8];
	g.shRGB_8[0] = floatToHalf(shc.x());
	g.shRGB_8[1] = floatToHalf(shc.y());
	g.shRGB_8[2] = floatToHalf(shc.z());

	buffer->unlock();

	const gridSize_t size = { 1, 1, 1 };
	return new IrradianceGrid(
		size,
		Aabb3(
			Vector4(-10000.0f, -10000.0f, -10000.0f),
			Vector4( 10000.0f,  10000.0f,  10000.0f)
		),
		buffer
	);
}

}
