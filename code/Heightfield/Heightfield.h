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
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Heightfield/HeightfieldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

/*!
 * \ingroup Heightfield
 */
class T_DLLCLASS Heightfield : public Object
{
	T_RTTI_CLASS;

public:
	explicit Heightfield(
		int32_t size,
		const Vector4& worldExtent
	);

	void setGridHeight(int32_t gridX, int32_t gridZ, float unitY);

	void setGridCut(int32_t gridX, int32_t gridZ, bool cut);

	void setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute);

	float getGridHeightNearest(int32_t gridX, int32_t gridZ) const;

	float getGridHeightBilinear(float gridX, float gridZ) const;

	float getWorldHeight(float worldX, float worldZ) const;

	bool getGridCut(int32_t gridX, int32_t gridZ) const;

	bool getWorldCut(float worldX, float worldZ) const;

	uint8_t getGridAttribute(int32_t gridX, int32_t gridZ) const;

	uint8_t getWorldAttribute(float worldX, float worldZ) const;

	void gridToWorld(int32_t gridX, int32_t gridZ, float& outWorldX, float& outWorldZ) const;

	void gridToWorld(float gridX, float gridZ, float& outWorldX, float& outWorldZ) const;

	Vector4 gridToWorld(float gridX, float gridZ) const;

	void worldToGrid(float worldX, float worldZ, int32_t& outGridX, int32_t& outGridZ) const;

	void worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const;

	float unitToWorld(float unitY) const;

	float worldToUnit(float worldY) const;

	Vector4 normalAt(float gridX, float gridZ) const;

	bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const;

	int32_t getSize() const { return m_size; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	height_t* getHeights() { return m_heights.ptr(); }

	const height_t* getHeights() const { return m_heights.c_ptr(); }

	uint8_t* getCuts() { return m_cuts.ptr(); }

	const uint8_t* getCuts() const { return m_cuts.c_ptr(); }

	uint8_t* getAttributes() { return m_attributes.ptr(); }

	const uint8_t* getAttributes() const { return m_attributes.c_ptr(); }

private:
	int32_t m_size;
	Vector4 m_worldExtent;
	float m_worldExtentFloats[4];
	AutoArrayPtr< height_t > m_heights;
	AutoArrayPtr< uint8_t > m_cuts;
	AutoArrayPtr< uint8_t > m_attributes;
};

}
