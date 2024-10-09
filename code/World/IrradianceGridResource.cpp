/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "World/IrradianceGrid.h"
#include "World/IrradianceGridResource.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.IrradianceGridResource", 0, IrradianceGridResource, ISerializable)

Ref< IrradianceGrid > IrradianceGridResource::createInstance(IStream* stream, render::IRenderSystem* renderSystem) const
{
	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 2)
	{
		log::error << L"Unable to read irradiance grid, unknown version " << version << Endl;
		return nullptr;
	}

	IrradianceGrid::gridSize_t size;
	reader >> size[0];
	reader >> size[1];
	reader >> size[2];

	float mn[3], mx[3];
	reader >> mn[0];
	reader >> mn[1];
	reader >> mn[2];
	reader >> mx[0];
	reader >> mx[1];
	reader >> mx[2];

	Ref< render::Buffer > buffer = renderSystem->createBuffer(
		render::BuStructured,
		size[0] * size[1] * size[2] * sizeof(IrradianceGridData),
		false
	);
	if (!buffer)
		return nullptr;

	IrradianceGridData* grid = (IrradianceGridData*)buffer->lock();
	T_FATAL_ASSERT(grid);

	float tmp[3];
	for (uint32_t x = 0; x < size[0]; ++x)
	{
		for (uint32_t y = 0; y < size[1]; ++y)
		{
			for (uint32_t z = 0; z < size[2]; ++z)
			{
				auto& g = *grid++;
				for (int32_t i = 0; i < 4; ++i)
				{
					reader >> tmp[0];
					reader >> tmp[1];
					reader >> tmp[2];
					g.shR0_3[i] = floatToHalf(tmp[0]);
					g.shG0_3[i] = floatToHalf(tmp[1]);
					g.shB0_3[i] = floatToHalf(tmp[2]);
				}
				for (int32_t i = 0; i < 4; ++i)
				{
					reader >> tmp[0];
					reader >> tmp[1];
					reader >> tmp[2];
					g.shR4_7[i] = floatToHalf(tmp[0]);
					g.shG4_7[i] = floatToHalf(tmp[1]);
					g.shB4_7[i] = floatToHalf(tmp[2]);
				}
				reader >> tmp[0];
				reader >> tmp[1];
				reader >> tmp[2];
				g.shRGB_8[0] = floatToHalf(tmp[0]);
				g.shRGB_8[1] = floatToHalf(tmp[1]);
				g.shRGB_8[2] = floatToHalf(tmp[2]);
			}
		}
	}

	buffer->unlock();

	return new IrradianceGrid(
		size,
		Aabb3(Vector4(mn[0], mn[1], mn[2]), Vector4(mx[0], mx[1], mx[2])),
		buffer
	);
}

void IrradianceGridResource::serialize(ISerializer& s)
{
}

}
