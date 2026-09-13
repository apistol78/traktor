/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb3.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Object.h"
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
	enum
	{
		//! Number of individual attribute layers.
		MaxAttributes = 16
	};

	explicit Heightfield(
		int32_t size,
		const Vector4& worldExtent);

	/*! Set extent of heightfield in world.
	 *
	 * Heights are stored normalized, thus changing the extent
	 * reinterprets the grid; cell bounds are updated accordingly.
	 */
	void setWorldExtent(const Vector4& worldExtent);

	void setGridHeight(int32_t gridX, int32_t gridZ, float unitY);

	void setGridCut(int32_t gridX, int32_t gridZ, bool cut);

	/*! Set density of a single attribute in a grid cell.
	 *
	 * Attributes are independent layers; setting one leave the others untouched.
	 * The layer is allocated on first use.
	 */
	void setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density);

	float getGridHeightNearest(int32_t gridX, int32_t gridZ) const;

	float getGridHeightNearestUnsafe(int32_t gridX, int32_t gridZ) const { return m_heights[gridX + gridZ * m_size] / 65535.0f; }

	float getGridHeightBilinear(float gridX, float gridZ) const;

	float getWorldHeight(float worldX, float worldZ) const;

	bool getGridCut(int32_t gridX, int32_t gridZ) const;

	bool getWorldCut(float worldX, float worldZ) const;

	//! Density of a single attribute in a grid cell, 0 to 255.
	uint8_t getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute) const;

	//! Bilinear filtered density of a single attribute at a world position, 0 to 1.
	float getWorldAttributeDensity(float worldX, float worldZ, uint8_t attribute) const;

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

	//! Allocate, and clear, an attribute layer unless it already exist.
	uint8_t* createAttributes(uint8_t attribute);

	//! Raw attribute layer; null when the layer has never been written.
	const uint8_t* getAttributes(uint8_t attribute) const;

	//! True if the attribute layer exist and contain any density.
	bool haveAttribute(uint8_t attribute) const;

	int32_t gridToCell(int32_t grid) const;

	void updateCellBounds();

	void updateCellBounds(int32_t gridX, int32_t gridZ);

	void updateCellBounds(int32_t gridX0, int32_t gridY0, int32_t gridX1, int32_t gridY1);

private:
	int32_t m_size;
	int32_t m_cellBoundsPitch;
	Vector4 m_worldExtent;
	float m_worldExtentFloats[4];
	AutoArrayPtr< height_t > m_heights;
	AutoArrayPtr< uint8_t > m_cuts;
	AutoArrayPtr< uint8_t > m_attributes[MaxAttributes];
	AutoArrayPtr< Aabb3 > m_cellBounds;
};

}
