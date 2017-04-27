/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Terrain/Editor/SymmetricalBrush.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SymmetricalBrush", SymmetricalBrush, IBrush)

SymmetricalBrush::SymmetricalBrush(const int32_t scale[2], const int32_t offset[2], IBrush* brush)
{
	m_scale[0] = scale[0];
	m_scale[1] = scale[1];
	m_offset[0] = offset[0];
	m_offset[1] = offset[1];
	m_brush[0] = brush;
	m_brush[1] = brush->clone();
}

uint32_t SymmetricalBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	uint32_t brushMode = 0;

	brushMode |= m_brush[0]->begin(
		x,
		y,
		radius,
		fallOff,
		strength,
		color,
		material
	);

	brushMode |= m_brush[1]->begin(
		m_offset[0] + x * m_scale[0],
		m_offset[1] + y * m_scale[1],
		radius,
		fallOff,
		strength,
		color,
		material
	);

	return brushMode;
}

void SymmetricalBrush::apply(int32_t x, int32_t y)
{
	m_brush[0]->apply(
		x,
		y
	);
	m_brush[1]->apply(
		m_offset[0] + x * m_scale[0],
		m_offset[1] + y * m_scale[1]
	);
}

void SymmetricalBrush::end(int32_t x, int32_t y)
{
	m_brush[0]->end(
		x,
		y
	);
	m_brush[1]->end(
		m_offset[0] + x * m_scale[0],
		m_offset[1] + y * m_scale[1]
	);
}

Ref< IBrush > SymmetricalBrush::clone() const
{
	return new SymmetricalBrush(
		m_scale,
		m_offset,
		m_brush[0]->clone()
	);
}

	}
}
