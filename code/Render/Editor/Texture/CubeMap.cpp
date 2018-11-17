/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Render/Editor/Texture/CubeMap.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeMap", CubeMap, Object)

CubeMap::CubeMap(int32_t size, const drawing::PixelFormat& pixelFormat)
:	m_size(size)
{
	for (int side = 0; side < 6; ++side)
	{
		m_side[side] = new drawing::Image(pixelFormat, m_size, m_size);
		m_side[side]->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

CubeMap::CubeMap(const drawing::Image* cubeMap)
:	m_size(0)
{
	int32_t width = cubeMap->getWidth();
	int32_t height = cubeMap->getHeight();

	int32_t layout = 0;
	m_size = height;

	if (height == width / 6)
	{
		// [+x][-x][+y][-y][+z][-z]
		layout = 0;
		m_size = height;
	}
	else if (height / 3 == width / 4)
	{
		// [  ][+y][  ][  ]
		// [-x][+z][+x][-z]
		// [  ][-y][  ][  ]
		layout = 1;
		m_size = height / 3;
	}
	else if (height / 4 == width / 3)
	{
		// [  ][+y][  ]
		// [-x][+z][+x]
		// [  ][-y][  ]
		// [  ][-z][  ]
		layout = 2;
		m_size = height / 4;
	}

	for (int side = 0; side < 6; ++side)
	{
		m_side[side] = new drawing::Image(cubeMap->getPixelFormat(), m_size, m_size);

		if (layout == 0)
			m_side[side]->copy(cubeMap, side * m_size, 0, m_size, m_size);
		else if (layout == 1)
		{
			const int32_t c_sideOffsets[][2] =
			{
				{ 2, 1 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 2 },
				{ 1, 1 },
				{ 3, 1 }
			};
			m_side[side]->copy(
				cubeMap,
				c_sideOffsets[side][0] * m_size,
				c_sideOffsets[side][1] * m_size,
				m_size,
				m_size
			);
		}
		else if (layout == 2)
		{
			const int32_t c_sideOffsets[][2] =
			{
				{ 2, 1 },
				{ 0, 1 },
				{ 1, 0 },
				{ 1, 2 },
				{ 1, 1 },
				{ 1, 3 }
			};
			m_side[side]->copy(
				cubeMap,
				c_sideOffsets[side][0] * m_size,
				c_sideOffsets[side][1] * m_size,
				m_size,
				m_size
			);
			if (side == 5)
			{
				// Flip -Z as it's defined up-side down in this layout.
				drawing::MirrorFilter filter(true, true);
				m_side[side]->apply(&filter);
			}
		}
	}
}

Ref< drawing::Image > CubeMap::createCrossImage() const
{
	Ref< drawing::Image > cross = new drawing::Image(m_side[0]->getPixelFormat(), m_size * 3, m_size * 4);
	cross->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	for (int32_t i = 0; i < 6; ++i)
	{
		const int32_t c_sideOffsets[][2] =
		{
			{ 2, 1 },
			{ 0, 1 },
			{ 1, 0 },
			{ 1, 2 },
			{ 1, 1 },
			{ 1, 3 }
		};

		Ref< drawing::Image > side = m_side[i];

		if (i == 5)
		{
			drawing::MirrorFilter filter(true, true);
			side = side->clone();
			side->apply(&filter);
		}

		cross->copy(
			side,
			c_sideOffsets[i][0] * m_size,
			c_sideOffsets[i][1] * m_size,
			0,
			0,
			m_size,
			m_size
		);
	}

	return cross;
}

Vector4 CubeMap::getDirection(int32_t side, int32_t x, int32_t y) const
{
	float fx = 2.0f * float(x) / m_size - 1.0f;
	float fy = 2.0f * float(y) / m_size - 1.0f;
	switch (side)
	{
	case 0:	// +x
		return Vector4(1.0f, -fy, -fx).normalized();

	case 1:	// -x
		return Vector4(-1.0f, -fy, fx).normalized();

	case 2:	// +y
		return Vector4(fx, 1.0f, fy).normalized();

	case 3:	// -y
		return Vector4(fx, -1.0f, -fy).normalized();

	case 4:	// +z
		return Vector4(fx, -fy, 1.0f).normalized();

	case 5:	// -z
		return Vector4(-fx, -fy, -1.0f).normalized();

	default:
		return Vector4::zero();
	}
}

void CubeMap::getPosition(const Vector4& direction, int32_t& outSide, int32_t& outX, int32_t& outY) const
{
	int32_t major = majorAxis3(direction);
	outSide = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

	float u, v, n;
	switch (major)
	{
	case 0:
		u = -sign(direction.x()) * direction.z();
		v = -direction.y();
		n = 1.0f / abs(direction.x());
		break;

	case 1:
		u = direction.x();
		v = sign(direction.y()) * direction.z();
		n = 1.0f / abs(direction.y());
		break;

	case 2:
		u = sign(direction.z()) * direction.x();
		v = -direction.y();
		n = 1.0f / abs(direction.z());
		break;
	}

	u *= n;
	v *= n;

	outX = clamp< int32_t >((u * 0.5f + 0.5f) * (m_size - 1), 0, m_size - 1);
	outY = clamp< int32_t >((v * 0.5f + 0.5f) * (m_size - 1), 0, m_size - 1);
}

void CubeMap::set(const Vector4& direction, const Color4f& value)
{
	int32_t major = majorAxis3(direction);
	int32_t side = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

	float u, v, n;
	switch (major)
	{
	case 0:
		u = -sign(direction.x()) * direction.z();
		v = -direction.y();
		n = 1.0f / abs(direction.x());
		break;

	case 1:
		u = direction.x();
		v = sign(direction.y()) * direction.z();
		n = 1.0f / abs(direction.y());
		break;

	case 2:
		u = sign(direction.z()) * direction.x();
		v = -direction.y();
		n = 1.0f / abs(direction.z());
		break;
	}

	u *= n;
	v *= n;

	int32_t x = clamp< int32_t >((u * 0.5f + 0.5f) * m_size, 0, m_size);
	int32_t y = clamp< int32_t >((v * 0.5f + 0.5f) * m_size, 0, m_size);

	m_side[side]->setPixel(x, y, value);
}

Color4f CubeMap::get(const Vector4& direction) const
{
	int32_t major = majorAxis3(direction);
	int32_t side = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

	float u, v, n;
	switch (major)
	{
	case 0:
		u = -sign(direction.x()) * direction.z();
		v = -direction.y();
		n = 1.0f / abs(direction.x());
		break;

	case 1:
		u = direction.x();
		v = sign(direction.y()) * direction.z();
		n = 1.0f / abs(direction.y());
		break;

	case 2:
		u = sign(direction.z()) * direction.x();
		v = -direction.y();
		n = 1.0f / abs(direction.z());
		break;
	}

	u *= n;
	v *= n;

	int32_t x = clamp< int32_t >((u * 0.5f + 0.5f) * m_size, 0, m_size);
	int32_t y = clamp< int32_t >((v * 0.5f + 0.5f) * m_size, 0, m_size);

	Color4f s;
	m_side[side]->getPixel(x, y, s);
	return s;
}

	}
}
