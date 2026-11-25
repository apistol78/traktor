/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ErosionFilter.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Random.h"
#include "Core/Math/Vector2.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace traktor::hf
{
namespace
{

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

const int32_t c_regionSize = 64;	//!< Samples along each side of a region.
const int32_t c_regionMargin = 16;	//!< Samples outside of region where droplet keep running; less than half of region minus footprint of a move.
const int32_t c_samplesPerDroplet = 512;	//!< Samples per droplet of a batch.
const int32_t c_minBatchSize = 1024;
const int32_t c_maxThreads = 64;

//! Water droplet, kept while it's moved between regions.
struct Droplet
{
	int32_t index;
	int32_t moves;
	int32_t xi, zi;
	float xp, zp, xf, zf;
	float h, h00, h10, h01, h11;
	float dx, dz;
	float s, v, w;
};

float& sample(float* hmap, int32_t size, int32_t x, int32_t z)
{
	return hmap[clamp(x, 0, size - 1) + clamp(z, 0, size - 1) * size];
}

//! Deposit sediment, bilinearly, around droplet.
void deposit(float* hmap, int32_t size, const Droplet& d, float ds)
{
	sample(hmap, size, d.xi, d.zi) += ds * (1.0f - d.xf) * (1.0f - d.zf);
	sample(hmap, size, d.xi + 1, d.zi) += ds * d.xf * (1.0f - d.zf);
	sample(hmap, size, d.xi, d.zi + 1) += ds * (1.0f - d.xf) * d.zf;
	sample(hmap, size, d.xi + 1, d.zi + 1) += ds * d.xf * d.zf;
}

int32_t regionOf(const Droplet& d, int32_t size, int32_t regions)
{
	return clamp(d.xi, 0, size - 1) / c_regionSize + (clamp(d.zi, 0, size - 1) / c_regionSize) * regions;
}

//! True if droplet is within margin of region; droplets oscillating at edge of region are thus kept running.
bool withinRegion(const Droplet& d, int32_t size, int32_t region, int32_t regions)
{
	const int32_t x = clamp(d.xi, 0, size - 1) - (region % regions) * c_regionSize;
	const int32_t z = clamp(d.zi, 0, size - 1) - (region / regions) * c_regionSize;
	return x >= -c_regionMargin && x < c_regionSize + c_regionMargin && z >= -c_regionMargin && z < c_regionSize + c_regionMargin;
}

//! Random number, 0 to 1, for move of droplet; independent of order droplets are run.
float random(int32_t index, int32_t move)
{
	uint32_t h = ((uint32_t)index * 0x9e3779b1u) ^ ((uint32_t)move * 0x85ebca77u);
	h ^= h >> 16;
	h *= 0x85ebca6bu;
	h ^= h >> 13;
	h *= 0xc2b2ae35u;
	h ^= h >> 16;
	return (h >> 8) * (1.0f / 16777216.0f);
}

/*! Move droplet until it's trapped, out of moves or beyond margin of region.
 * \return True if droplet is finished, false if it continue in another region.
 */
bool run(Droplet& d, float* hmap, int32_t size, int32_t maxMoves, int32_t region, int32_t regions)
{
	// Heights are read when droplet start or resume, since other droplets may have changed them since it was paused.
	d.h00 = sample(hmap, size, d.xi, d.zi);
	d.h10 = sample(hmap, size, d.xi + 1, d.zi);
	d.h01 = sample(hmap, size, d.xi, d.zi + 1);
	d.h11 = sample(hmap, size, d.xi + 1, d.zi + 1);
	d.h = (d.h00 * (1.0f - d.xf) + d.h10 * d.xf) * (1.0f - d.zf) + (d.h01 * (1.0f - d.xf) + d.h11 * d.xf) * d.zf;

	for (; d.moves < maxMoves; ++d.moves)
	{
		if (!withinRegion(d, size, region, regions))
			return false;

		const float gx = d.h00 + d.h01 - d.h10 - d.h11;
		const float gz = d.h00 + d.h10 - d.h01 - d.h11;

		d.dx = (d.dx - gx) * Ki + gx;
		d.dz = (d.dz - gz) * Ki + gz;

		const float dl = std::sqrt(d.dx * d.dx + d.dz * d.dz);
		if (dl <= FUZZY_EPSILON)
		{
			const float a = random(d.index, d.moves) * TWO_PI;
			d.dx = std::cos(a);
			d.dz = std::sin(a);
		}
		else
		{
			d.dx /= dl;
			d.dz /= dl;
		}

		const float nxp = d.xp + d.dx;
		const float nzp = d.zp + d.dz;
		const int32_t nxi = (int32_t)nxp;
		const int32_t nzi = (int32_t)nzp;
		const float nxf = nxp - nxi;
		const float nzf = nzp - nzi;

		const float nh00 = sample(hmap, size, nxi, nzi);
		const float nh10 = sample(hmap, size, nxi + 1, nzi);
		const float nh01 = sample(hmap, size, nxi, nzi + 1);
		const float nh11 = sample(hmap, size, nxi + 1, nzi + 1);
		const float nh = (nh00 * (1.0f - nxf) + nh10 * nxf) * (1.0f - nzf) + (nh01 * (1.0f - nxf) + nh11 * nxf) * nzf;

		// Moving uphill fill pit with sediment; droplet stops if it hasn't enough to fill it.
		if (nh >= d.h)
		{
			const float ds = (nh - d.h) + 0.001f;
			if (ds >= d.s)
			{
				deposit(hmap, size, d, d.s);
				return true;
			}

			deposit(hmap, size, d, ds);
			d.h += ds;
			d.s -= ds;
			d.v = 0.0f;
		}

		float dh = d.h - nh;
		float ds = d.s - std::max(dh, minSlope) * d.v * d.w * Kq;
		if (ds >= 0.0f)
		{
			ds *= Kd;
			deposit(hmap, size, d, ds);
			dh += ds;
			d.s -= ds;
		}
		else
		{
			ds = std::min(ds * -Kr, dh * 0.99f);
			for (int32_t z = d.zi - 1; z <= d.zi + 2; ++z)
			{
				const float zo = z - d.zp;
				for (int32_t x = d.xi - 1; x <= d.xi + 2; ++x)
				{
					const float xo = x - d.xp;
					const float w = 1.0f - (xo * xo + zo * zo) * 0.25f;
					if (w > 0.0f)
						sample(hmap, size, x, z) -= ds * w * 0.1591549430918953f;
				}
			}
			dh -= ds;
			d.s += ds;
		}

		d.v = std::sqrt(d.v * d.v + Kg * dh);
		d.w *= 1.0f - Kw;

		d.xp = nxp;
		d.zp = nzp;
		d.xi = nxi;
		d.zi = nzi;
		d.xf = nxf;
		d.zf = nzf;

		d.h = nh;
		d.h00 = nh00;
		d.h10 = nh10;
		d.h01 = nh01;
		d.h11 = nh11;
	}

	return true;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ErosionFilter", ErosionFilter, Object)

ErosionFilter::ErosionFilter(int32_t iterations)
:	m_iterations(iterations)
{
}

void ErosionFilter::apply(Heightfield* heightfield) const
{
	const int32_t size = heightfield->getSize();
	const int32_t regions = (size + c_regionSize - 1) / c_regionSize;
	const int32_t maxMoves = size * 4;
	const int32_t threadCount = clamp((int32_t)OS::getInstance().getCPUCoreCount(), 1, c_maxThreads);

	AlignedVector< float > hmap((size_t)size * size);
	for (int32_t z = 0; z < size; ++z)
		for (int32_t x = 0; x < size; ++x)
			hmap[x + (size_t)z * size] = heightfield->unitToWorld(heightfield->getGridHeightNearestUnsafe(x, z));

	// Droplets of a batch run simultaneously, so batch is kept sparse for droplets to rarely share paths; batches are run in order.
	const int32_t batchSize = (int32_t)std::max< int64_t >((int64_t)size * size / c_samplesPerDroplet, c_minBatchSize);

	AlignedVector< AlignedVector< Droplet > > bins(regions * regions);
	AlignedVector< AlignedVector< Droplet > > left(regions * regions);
	AlignedVector< int32_t > active;
	Random rnd;
	for (int32_t batch = 0; batch < m_iterations; batch += batchSize)
	{
		// Droplets start at random positions, in order, in region of their start.
		for (int32_t i = batch; i < std::min(batch + batchSize, m_iterations); ++i)
		{
			const Vector2 ruv = Quasirandom::hammersley(i, m_iterations, rnd);
			Droplet d = {};
			d.index = i;
			d.xi = (int32_t)(ruv.x * (size - 1));
			d.zi = (int32_t)(ruv.y * (size - 1));
			d.xp = (float)d.xi;
			d.zp = (float)d.zi;
			d.w = 1.0f;
			bins[regionOf(d, size, regions)].push_back(d);
		}

		// Regions of same parity are apart, so their droplets are run in parallel; droplets beyond margin of a region continue in a later phase.
		// Pool threads are used since filter is run by job workers which other workers wait on.
		for (bool pending = true; pending;)
		{
			for (int32_t parity = 0; parity < 4; ++parity)
			{
				active.resize(0);
				for (int32_t r = 0; r < regions * regions; ++r)
					if (!bins[r].empty() && ((r % regions) & 1) + ((r / regions) & 1) * 2 == parity)
						active.push_back(r);

				std::atomic< int32_t > next = 0;
				const ThreadPool::threadPoolFn_t runRegions = [&]() {
					for (int32_t i = next++; i < (int32_t)active.size(); i = next++)
					{
						const int32_t r = active[i];
						for (Droplet& d : bins[r])
							if (!run(d, hmap.ptr(), size, maxMoves, r, regions))
								left[r].push_back(d);
						bins[r].resize(0);
					}
				};

				Thread* threads[c_maxThreads] = { nullptr };
				const int32_t spawnCount = std::min(threadCount, (int32_t)active.size()) - 1;
				for (int32_t i = 0; i < spawnCount; ++i)
					ThreadPool::getInstance().spawn(runRegions, threads[i]);
				runRegions();
				for (int32_t i = 0; i < spawnCount; ++i)
					ThreadPool::getInstance().join(threads[i]);

				for (const int32_t r : active)
				{
					for (const Droplet& d : left[r])
						bins[regionOf(d, size, regions)].push_back(d);
					left[r].resize(0);
				}
			}

			pending = false;
			for (const auto& bin : bins)
				pending |= !bin.empty();
		}
	}

	for (int32_t z = 0; z < size; ++z)
		for (int32_t x = 0; x < size; ++x)
			heightfield->setGridHeight(x, z, heightfield->worldToUnit(hmap[x + (size_t)z * size]));

	heightfield->updateCellBounds();
}

}
