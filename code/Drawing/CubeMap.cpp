/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/CubeMap.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.CubeMap", CubeMap, Object)

CubeMap::CubeMap(int32_t size, const drawing::PixelFormat& pixelFormat)
:	m_size(size)
{
	for (int side = 0; side < 6; ++side)
	{
		m_side[side] = new drawing::Image(pixelFormat, m_size, m_size);
		m_side[side]->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

CubeMap::CubeMap(const Ref< drawing::Image > sides[6])
{
	for (int32_t i = 0; i < 6; ++i)
		m_side[i] = sides[i];

	m_size = sides[0]->getWidth();
}

Ref< CubeMap > CubeMap::createFromCrossImage(const drawing::Image* image)
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();

	int32_t layout = 0;
	int32_t size = height;

	if (height == width / 6)
	{
		// [+x][-x][+y][-y][+z][-z]
		layout = 0;
		size = height;
	}
	else if (height / 3 == width / 4)
	{
		// [  ][+y][  ][  ]
		// [-x][+z][+x][-z]
		// [  ][-y][  ][  ]
		layout = 1;
		size = height / 3;
	}
	else if (height / 4 == width / 3)
	{
		// [  ][+y][  ]
		// [-x][+z][+x]
		// [  ][-y][  ]
		// [  ][-z][  ]
		layout = 2;
		size = height / 4;
	}

	Ref< drawing::Image > images[6];
	for (int32_t side = 0; side < 6; ++side)
	{
		images[side] = new drawing::Image(image->getPixelFormat(), size, size);

		if (layout == 0)
			images[side]->copy(image, side * size, 0, size, size);
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
			images[side]->copy(
				image,
				c_sideOffsets[side][0] * size,
				c_sideOffsets[side][1] * size,
				size,
				size
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
			images[side]->copy(
				image,
				c_sideOffsets[side][0] * size,
				c_sideOffsets[side][1] * size,
				size,
				size
			);
			if (side == 5)
			{
				// Flip -Z as it's defined up-side down in this layout.
				drawing::MirrorFilter filter(true, true);
				images[side]->apply(&filter);
			}
		}
	}

	return new CubeMap(images);
}

Ref< CubeMap > CubeMap::createFromEquirectangularImage(const drawing::Image* image)
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();
	const int32_t size = height;

	Ref< CubeMap > cubeMap = new CubeMap(size, image->getPixelFormat());	

    const float an = std::sin(PI / 4.0f);
    const float ak = std::cos(PI / 4.0f);

	const float c_faceTransform[6][2] = 
	{ 
		{ 0.0f, 0.0f },
		{ PI, 0.0f },
		{ 0.0f, -PI / 2.0f },
		{ 0.0f, PI / 2.0f },
		{ -PI / 2.0f, 0.0f },
		{ PI / 2.0f, 0.0f }
	};

	for (int32_t side = 0; side < 6; ++side)
	{
    	const float ftu = c_faceTransform[side][0];
    	const float ftv = c_faceTransform[side][1];

		for (int32_t y = 0; y < size; ++y)
		{
			for (int32_t x = 0; x < size; ++x)
			{
				float nx = 2.0f * ((float)x / (float)size - 0.5f);
				float ny = 2.0f * ((float)y / (float)size - 0.5f);

				nx *= an; 
				ny *= an; 

				float u, v;
				if (ftv == 0.0f)
				{
					u = std::atan2(nx, ak);
					v = std::atan2(ny * std::cos(u), ak);
					u += ftu; 
				}
				else if(ftv > 0.0f)
				{ 
					float d = std::sqrt(nx * nx + ny * ny);
					v = PI / 2 - std::atan2(d, ak);
					u = std::atan2(ny, nx);
				}
				else
				{
					float d = std::sqrt(nx * nx + ny * ny);
					v = -PI / 2 + std::atan2(d, ak);
					u = std::atan2(-ny, nx);
				}

				u = u / (PI); 
				v = v / (PI / 2.0f);

				while (v < -1.0f)
				{
					v += 2.0f;
					u += 1.0f;
				} 
				while (v > 1.0f)
				{
					v -= 2.0f;
					u += 1.0f;
				} 

				while (u < -1.0f)
					u += 2.0f;
				while (u > 1.0f)
					u -= 2;

				u = u / 2.0f + 0.5f;
				v = v / 2.0f + 0.5f;

				u = u * (width - 1);
				v = v * (height - 1);

				Color4f cl;
				image->getPixel((int32_t)u, (int32_t)v, cl);
				cubeMap->getSide(side)->setPixel(x, y, cl);
			}
		}
	}

	return cubeMap;
}

Ref< CubeMap > CubeMap::createFromImage(const drawing::Image* image)
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();
	
	if (width / 4 == height || width / 2 == height)
		return CubeMap::createFromEquirectangularImage(image);
	else
		return CubeMap::createFromCrossImage(image);
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

Ref< drawing::Image > CubeMap::createEquirectangular() const
{
	Ref< drawing::Image > output = new drawing::Image(m_side[0]->getPixelFormat(), m_size * 4, m_size);
	for (int32_t y = 0; y < output->getHeight(); ++y)
	{
		const float fy = (float)y / (output->getHeight() - 1);
		for (int32_t x = 0; x < output->getWidth(); ++x)
		{
			const float fx = (float)x / (output->getWidth() - 1);
			const float phi = fx * TWO_PI;
			const float theta = fy * PI;
			const float dx = sin(phi) * sin(theta);
			const float dy = cos(theta);
			const float dz = cos(phi) * sin(theta);

			Color4f c = get(Vector4(dx, dy, dz));
			output->setPixel(x, y, c);
		}
	}
	return output;
}

Vector4 CubeMap::getDirection(int32_t side, int32_t x, int32_t y) const
{
	const float fx = 2.0f * float(x) / m_size - 1.0f;
	const float fy = 2.0f * float(y) / m_size - 1.0f;
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
	const int32_t major = majorAxis3(direction);
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
			
	default:
		outX = outY = 0;
		return;
	}

	u *= n;
	v *= n;

	outX = clamp< int32_t >((u * 0.5f + 0.5f) * (m_size - 1), 0, m_size - 1);
	outY = clamp< int32_t >((v * 0.5f + 0.5f) * (m_size - 1), 0, m_size - 1);
}

void CubeMap::set(const Vector4& direction, const Color4f& value)
{
	const int32_t major = majorAxis3(direction);
	const int32_t side = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

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
            
    default:
        return;
	}

	u *= n;
	v *= n;

	const int32_t s = m_size - 1;
	const int32_t x = clamp< int32_t >((u * 0.5f + 0.5f) * s, 0, s);
	const int32_t y = clamp< int32_t >((v * 0.5f + 0.5f) * s, 0, s);

	m_side[side]->setPixel(x, y, value);
}

Color4f CubeMap::get(const Vector4& direction) const
{
	const int32_t major = majorAxis3(direction);
	const int32_t side = major * 2 + (direction[major] >= 0.0f ? 0 : 1);

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
			
	default:
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	u *= n;
	v *= n;

	const int32_t s = m_size - 1;
	const int32_t x = clamp< int32_t >((u * 0.5f + 0.5f) * s, 0, s);
	const int32_t y = clamp< int32_t >((v * 0.5f + 0.5f) * s, 0, s);

	Color4f color;
	if (m_side[side]->getPixel(x, y, color))
		return color;
	else
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
}

}
