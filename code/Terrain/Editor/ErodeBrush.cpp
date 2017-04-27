/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/IFallOff.h"
#include "Terrain/Editor/ErodeBrush.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const struct Flow
{
	int32_t x;
	int32_t y;
	Vector2 d;
}
c_flow[8] =
{
	{ -1, -1, Vector2(-0.71f, -0.71f) },
	{  0, -1, Vector2(  0.0f,  -1.0f) },
	{  1, -1, Vector2( 0.71f, -0.71f) },
	{ -1,  0, Vector2( -1.0f,   0.0f) },
	{  1,  0, Vector2(  1.0f,   0.0f) },
	{ -1,  1, Vector2(-0.71f,  0.71f) },
	{  0,  1, Vector2(  0.0f,   1.0f) },
	{  1,  1, Vector2( 0.71f,  0.71f) }
};

Vector4 normalAt(const hf::Heightfield* heightfield, int32_t u, int32_t v)
{
	const float c_distance = 0.5f;
	const float directions[][2] =
	{
		{ -c_distance, -c_distance },
		{        0.0f, -c_distance },
		{  c_distance, -c_distance },
		{  c_distance,        0.0f },
		{  c_distance,  c_distance },
		{        0.0f,        0.0f },
		{ -c_distance,  c_distance },
		{ -c_distance,        0.0f }
	};

	float h0 = heightfield->getGridHeightNearest(u, v);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = heightfield->getGridHeightBilinear(u + directions[i][0], v + directions[i][1]);

	const Vector4& worldExtent = heightfield->getWorldExtent();
	float sx = worldExtent.x() / heightfield->getSize();
	float sy = worldExtent.y();
	float sz = worldExtent.z() / heightfield->getSize();

	Vector4 N = Vector4::zero();

	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		uint32_t j = (i + 1) % sizeof_array(directions);

		float dx1 = directions[i][0] * sx;
		float dy1 = (h[i] - h0) * sy;
		float dz1 = directions[i][1] * sz;

		float dx2 = directions[j][0] * sx;
		float dy2 = (h[j] - h0) * sy;
		float dz2 = directions[j][1] * sz;

		Vector4 n = cross(
			Vector4(dx2, dy2, dz2),
			Vector4(dx1, dy1, dz1)
		);

		N += n;
	}

	return N.normalized();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ErodeBrush", ErodeBrush, IBrush)

ErodeBrush::ErodeBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
	if (heightfield)
	{
		int32_t size = heightfield->getSize();
		m_water.reset(new WaterCell [size * size]);
	}
}

uint32_t ErodeBrush::begin(int32_t x, int32_t y, int32_t radius, const IFallOff* fallOff, float strength, const Color4f& color, int32_t material)
{
	m_radius = radius;
	m_fallOff = fallOff;
	m_strength = powf(abs(strength), 3.0f);

	int32_t size = m_heightfield->getSize();
	for (int32_t i = 0; i < size; ++i)
	{
		m_water[i].level = 0.0f;
		m_water[i].velocity = Vector2(0.0f, 0.0f);
	}

	return MdHeight;
}

void ErodeBrush::apply(int32_t x, int32_t y)
{
	int32_t size = m_heightfield->getSize();

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			int32_t iix = ix + x;
			int32_t iiy = iy + y;

			if (iix >= 0 && iiy >= 0 && iix < size && iiy < size)
				m_water[iix + iiy * size].level += a;
		}
	}

	int32_t fromX = max(x - m_radius - 1, 1);
	int32_t toX = min(x + m_radius + 2, size - 1);
	int32_t fromY = max(y - m_radius - 1, 1);
	int32_t toY = min(y + m_radius + 2, size - 1);

	for (int32_t N = 0; N < 100; ++N)
	{
		int32_t move = 0;

		for (int32_t iy = fromY; iy < toY; ++iy)
		{
			for (int32_t ix = fromX; ix < toX; ++ix)
			{
				WaterCell& w0 = m_water[ix + iy * size];
				if (w0.level <= 0.01f)
					continue;

				Vector4 normal = normalAt(m_heightfield, ix, iy);
				Vector2 direction(normal.x(), normal.z());

				for (int32_t i = 0; i < 8; ++i)
				{
					float a = dot(direction, c_flow[i].d);
					if (a > 0.0f)
					{
						T_FATAL_ASSERT(a < 1.0f + FUZZY_EPSILON);

						WaterCell& w1 = m_water[(ix + c_flow[i].x) + (iy + c_flow[i].y) * size];

						float m = min(w0.level * a, 1.0f);
						T_ASSERT (m > 0.0f);

						w0.level = max(w0.level - m, 0.0f);
						w1.level = min(w1.level + m, 20.0f);

						w0.velocity += direction * m * 0.01f;
						w1.velocity += direction * m * 0.0025f;

						++move;
					}
				}
			}
		}

		if (move <= 0)
			break;

		fromX = max(fromX - 1, 1);
		toX = min(toX + 1, size - 1);
		fromY = max(fromY - 1, 1);
		toY = min(toY + 1, size - 1);
	}

	for (int32_t iy = fromY; iy < toY; ++iy)
	{
		for (int32_t ix = fromX; ix < toX; ++ix)
		{
			WaterCell& w0 = m_water[ix + iy * size];
			if (w0.velocity.length2() <= FUZZY_EPSILON)
				continue;

			// Measure total magnitude of neighbors velocities into this cell.
			float totalVelocityIn = 0.0f;
			for (int32_t i = 0; i < 8; ++i)
			{
				Vector2 velocityIn = m_water[(ix + c_flow[i].x) + (iy + c_flow[i].y) * size].velocity;
				float Din = -dot(velocityIn, c_flow[i].d);
				if (Din > 0.0f)
					totalVelocityIn += Din;
			}
			if (totalVelocityIn <= FUZZY_EPSILON)
				continue;

			// Move masses from this cell to neighbors.
			Vector2 velocity = w0.velocity.normalized() * totalVelocityIn;

			float hu0 = m_heightfield->getGridHeightNearest(ix, iy);
			float hw0 = m_heightfield->unitToWorld(hu0);

			for (int32_t i = 0; i < 8; ++i)
			{
				float Dout = dot(velocity, c_flow[i].d);
				if (Dout > 0.0f)
				{
					float hu1 = m_heightfield->getGridHeightNearest(ix + c_flow[i].x, iy + c_flow[i].y);
					float hw1 = m_heightfield->unitToWorld(hu1);

					hw0 -= Dout * 0.1f;
					hw1 += Dout * 0.1f;

					hu1 = m_heightfield->worldToUnit(hw1);
					m_heightfield->setGridHeight(ix + c_flow[i].x, iy + c_flow[i].y, clamp(hu1, 0.0f, 1.0f));
				}
			}

			hu0 = m_heightfield->worldToUnit(hw0);
			m_heightfield->setGridHeight(ix, iy, clamp(hu0, 0.0f, 1.0f));
		}
	}
}

void ErodeBrush::end(int32_t x, int32_t y)
{
}

Ref< IBrush > ErodeBrush::clone() const
{
	return new ErodeBrush(m_heightfield);
}

	}
}
