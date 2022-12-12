/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Random.h"
#include "Core/Math/Vector2.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/ErosionFilter.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ErosionFilter", ErosionFilter, Object)

ErosionFilter::ErosionFilter(int32_t iterations)
:	m_iterations(iterations)
{
}

void ErosionFilter::apply(Heightfield* heightfield) const
{
	const int32_t size = heightfield->getSize();

	/*

http://ranmantaru.com/blog/2011/10/08/water-erosion-on-heightmap-terrain/
	
Kq and minSlope are for soil carry capacity (see the formula above).
Kw is water evaporation speed.
Kr is erosion speed (how fast the soil is removed).
Kd is deposition speed (how fast the extra sediment is dropped).
Ki is direction inertia. Higher values make channel turns smoother.
g is gravity that accelerates the flows.
	
	*/
	const float Kq = 10.0f;
	const float Kw = 0.001f;
	const float Kr = 0.9f;
	const float Kd = 0.02f;
	const float Ki = 0.1f;
	const float minSlope = 0.05f;
	const float Kg = 20.0f * 2.0f;

	AlignedVector< float > hmap(size * size, 0.0f);

	for (int32_t y = 0; y < size; ++y)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			float h = heightfield->getGridHeightNearest(x, y);
			hmap[x + y * size] = heightfield->unitToWorld(h);
		}
	}

	AlignedVector< Vector2 > erosion(size * size, Vector2(0.0f, 0.0f));

	static const int32_t MAX_PATH_LEN = size*4;

	#define HMAP_INDEX(X, Z) (clamp((X), 0, size - 1) + clamp((Z), 0, size - 1) * size)
	#define HMAP(X, Z) hmap[HMAP_INDEX((X), (Z))]

	#define DEPOSIT_AT(X, Z, W) \
	{ \
		float delta=ds*(W); \
		erosion[HMAP_INDEX((X), (Z))].y += delta; \
		hmap   [HMAP_INDEX((X), (Z))]   += delta; \
	}

	#define DEPOSIT(H) \
		DEPOSIT_AT(xi  , zi  , (1-xf)*(1-zf)) \
		DEPOSIT_AT(xi+1, zi  ,    xf *(1-zf)) \
		DEPOSIT_AT(xi  , zi+1, (1-xf)*   zf ) \
		DEPOSIT_AT(xi+1, zi+1,    xf *   zf ) \
		(H)+=ds;

	Random rnd;

	for (int32_t iter = 0; iter < m_iterations; ++iter)
	{
		Vector2 ruv = Quasirandom::hammersley(iter, m_iterations, rnd);

		int32_t xi = (int32_t)(ruv.x * (size - 1));
		int32_t zi = (int32_t)(ruv.y * (size - 1));

		float xp = (float)xi;
		float zp = (float)zi;
		float xf = 0.0f;
		float zf = 0.0f;

		float h = HMAP(xi, zi);
		float s = 0.0f, v = 0.0f, w = 1.0f;

		float h00 = h;
		float h10 = HMAP(xi+1, zi  );
		float h01 = HMAP(xi  , zi+1);
		float h11 = HMAP(xi+1, zi+1);

		float dx = 0.0f, dz = 0.0f;

		int32_t numMoves = 0;
		for (; numMoves < MAX_PATH_LEN; ++numMoves)
		{
			float gx = h00 + h01 - h10 - h11;
			float gz = h00 + h10 - h01 - h11;

			dx = (dx - gx) * Ki + gx;
			dz = (dz - gz) * Ki + gz;

			float dl = sqrtf(dx * dx + dz * dz);
			if (dl <= FUZZY_EPSILON)
			{
				float a = rnd.nextFloat() * TWO_PI;
				dx = cosf(a);
				dz = sinf(a);
			}
			else
			{
				dx /= dl;
				dz /= dl;
			}

			float nxp = xp + dx;
			float nzp = zp + dz;

			int32_t nxi = (int32_t)(nxp);
			int32_t nzi = (int32_t)(nzp);
			float nxf = nxp - nxi;
			float nzf = nzp - nzi;

			float nh00 = HMAP(nxi  , nzi  );
			float nh10 = HMAP(nxi+1, nzi  );
			float nh01 = HMAP(nxi  , nzi+1);
			float nh11 = HMAP(nxi+1, nzi+1);

			float nh = (nh00 * (1.0f - nxf) + nh10 * nxf) * (1.0f - nzf) + (nh01 * (1.0f - nxf) + nh11 * nxf) * nzf;

			if (nh >= h)
			{
				float ds = (nh - h) + 0.001f;
				if (ds >= s)
				{
					ds = s;
					DEPOSIT(h)
					s = 0;
					break;
				}

				DEPOSIT(h)
				s -= ds;
				v = 0;
			}

			float dh = h - nh;
			float slope = dh;

			float q = max(slope, minSlope) * v * w * Kq;

			float ds = s - q;
			if (ds >= 0.0f)
			{
				ds *= Kd;
				DEPOSIT(dh)
				s -= ds;
			}
			else
			{
				ds *= -Kr;
				ds = min(ds, dh * 0.99f);

				for (int32_t z = zi - 1; z <= zi + 2; ++z)
				{
					float zo = z - zp;
					float zo2 = zo * zo;

					for (int32_t x = xi - 1; x <= xi + 2; ++x)
					{
						float xo = x - xp;

						float w = 1.0f - (xo * xo + zo2) * 0.25f;
						if (w <= 0.0f)
							continue;

						w *= 0.1591549430918953f;

						float delta = ds * w;
						hmap[HMAP_INDEX(x, z)] -= delta;
						Vector2& e = erosion[HMAP_INDEX(x, z)];
						float r = e.x, d = e.y;
						if (delta <= d)
							d -= delta;
						else
						{
							r += delta - d;
							d = 0.0f;
						}
						e.x = r;
						e.y = d;
					}
				}

				dh -= ds;
				s += ds;
			}

			v = sqrtf(v * v + Kg * dh);
			w *= 1.0f - Kw;

			xp = nxp;
			zp = nzp;
			xi = nxi;
			zi = nzi;
			xf = nxf;
			zf = nzf;

			h = nh;
			h00 = nh00;
			h10 = nh10;
			h01 = nh01;
			h11 = nh11;
		}
	}

	for (int32_t y = 0; y < size; ++y)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			float h = hmap[x + y * size];
			heightfield->setGridHeight(x, y, heightfield->worldToUnit(h));
		}
	}
}

}
