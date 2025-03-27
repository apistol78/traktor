/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/IrradianceGrid.h"

#include "Core/Math/Float.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/SH/SHCoeffs.h"

#include <cstring>

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGrid", IrradianceGrid, Object)

IrradianceGrid::IrradianceGrid(
	const gridSize_t size,
	const Aabb3& boundingBox,
	render::Buffer* buffer)
	: m_boundingBox(boundingBox)
	, m_buffer(buffer)
{
	std::memcpy(m_size, size, sizeof(gridSize_t));
}

Ref< IrradianceGrid > IrradianceGrid::createSingle(render::IRenderSystem* renderSystem, const render::SHCoeffs& shCoeffs)
{
	Ref< render::Buffer > buffer = renderSystem->createBuffer(
		render::BuStructured,
		sizeof(IrradianceGridData),
		false);
	if (!buffer)
		return nullptr;

	IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
	T_FATAL_ASSERT(grid);

	auto& g = *grid;

	const half_t halfZero = floatToHalf(0.0f);
	for (int32_t i = 0; i < 4; ++i)
	{
		const Vector4& shc = shCoeffs[i];
		g.shR0_3[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.x()) : halfZero;
		g.shG0_3[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.y()) : halfZero;
		g.shB0_3[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.z()) : halfZero;
	}
	for (int32_t i = 0; i < 4; ++i)
	{
		const Vector4& shc = shCoeffs[4 + i];
		g.shR4_7[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.x()) : halfZero;
		g.shG4_7[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.y()) : halfZero;
		g.shB4_7[i] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.z()) : halfZero;
	}

	const Vector4& shc = shCoeffs[8];
	g.shRGB_8[0] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.x()) : halfZero;
	g.shRGB_8[1] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.y()) : halfZero;
	g.shRGB_8[2] = !isNanOrInfinite(shc.x()) ? floatToHalf(shc.z()) : halfZero;

	buffer->unlock();

	const gridSize_t size = { 1, 1, 1 };
	return new IrradianceGrid(
		size,
		Aabb3(
			Vector4(-10000.0f, -10000.0f, -10000.0f),
			Vector4(10000.0f, 10000.0f, 10000.0f)),
		buffer);
}

}
