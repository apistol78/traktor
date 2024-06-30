/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Half.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class SHCoeffs;

}

namespace traktor::world
{

#pragma pack(1)

/*! Irradiance grid cell data.
 * \ingroup World
 */
struct IrradianceGridData
{
	float shR0_3[4];
	float shR4_7[4];
	float shG0_3[4];
	float shG4_7[4];
	float shB0_3[4];
	float shB4_7[4];
	float shRGB_8[4];
};

#pragma pack()

/*! Irradiance grid.
 * \ingroup World
 */
class T_DLLCLASS IrradianceGrid : public Object
{
	T_RTTI_CLASS;

public:
	typedef uint32_t gridSize_t [3];

	explicit IrradianceGrid(
		const gridSize_t size,
		const Aabb3& boundingBox,
		render::Buffer* buffer
	);

	const gridSize_t& getSize() const { return m_size; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

	render::Buffer* getBuffer() const { return m_buffer; }

	bool isSingle() const { return m_size[0] == 1 && m_size[1] == 1 && m_size[2] == 1; }

	static Ref< IrradianceGrid > createSingle(render::IRenderSystem* renderSystem, const render::SHCoeffs& shCoeffs);

private:
	gridSize_t m_size;
	Aabb3 m_boundingBox;
	Ref< render::Buffer > m_buffer;
};

}
