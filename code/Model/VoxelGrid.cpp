/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Model/VoxelGrid.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.VoxelGrid", VoxelGrid, Object)

VoxelGrid::VoxelGrid(int32_t sizeX, int32_t sizeY, int32_t sizeZ)
:	m_sizeX(sizeX)
,	m_sizeY(sizeY)
,	m_sizeZ(sizeZ)
{
	m_cells.resize(sizeX * sizeY * sizeZ, 0);
}

void VoxelGrid::set(int32_t x, int32_t y, int32_t z, uint32_t value)
{
	if (x < 0 || x >= m_sizeX)
		return;
	if (y < 0 || y >= m_sizeY)
		return;
	if (z < 0 || z >= m_sizeZ)
		return;

	uint32_t offset = z * m_sizeX * m_sizeY + y * m_sizeX + x;
	m_cells[offset] = value;
}

uint32_t VoxelGrid::get(int32_t x, int32_t y, int32_t z) const
{
	if (x < 0 || x >= m_sizeX)
		return 0;
	if (y < 0 || y >= m_sizeY)
		return 0;
	if (z < 0 || z >= m_sizeZ)
		return 0;

	uint32_t offset = z * m_sizeX * m_sizeY + y * m_sizeX + x;
	return m_cells[offset];
}

	}
}
