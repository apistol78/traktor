/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Packer.h"

#include "Core/Containers/AlignedVector.h"

#include <limits>

namespace traktor::spark
{

class PackerImpl
{
public:
	explicit PackerImpl(int32_t width, int32_t height)
		: m_width(width)
		, m_height(height)
	{
		reset();
	}

	bool insert(int32_t width, int32_t height, Packer::Rectangle& outRectangle)
	{
		if (width <= 0 || height <= 0)
			return false;

		// Best short side fit; leaves the remainder as square as it can, which packs
		// the mixed sizes an atlas see better than taking the first rectangle that fit.
		int32_t best = -1;
		int32_t bestShort = std::numeric_limits< int32_t >::max();
		int32_t bestLong = std::numeric_limits< int32_t >::max();

		for (int32_t i = 0; i < (int32_t)m_free.size(); ++i)
		{
			const Packer::Rectangle& f = m_free[i];
			if (f.width < width || f.height < height)
				continue;

			const int32_t dw = f.width - width;
			const int32_t dh = f.height - height;
			const int32_t shortFit = std::min(dw, dh);
			const int32_t longFit = std::max(dw, dh);

			if (shortFit < bestShort || (shortFit == bestShort && longFit < bestLong))
			{
				best = i;
				bestShort = shortFit;
				bestLong = longFit;
			}
		}

		if (best < 0)
			return false;

		const Packer::Rectangle node = m_free[best];
		m_free.erase(m_free.begin() + best);

		outRectangle = { node.x, node.y, width, height };
		m_allocated++;

		// Guillotine split of what's left over; cut along the shorter leftover so the
		// two pieces stay as usable as possible.
		const int32_t dw = node.width - width;
		const int32_t dh = node.height - height;
		if (dw < dh)
		{
			if (dw > 0)
				m_free.push_back({ node.x + width, node.y, dw, height });
			if (dh > 0)
				m_free.push_back({ node.x, node.y + height, node.width, dh });
		}
		else
		{
			if (dh > 0)
				m_free.push_back({ node.x, node.y + height, width, dh });
			if (dw > 0)
				m_free.push_back({ node.x + width, node.y, dw, node.height });
		}

		return true;
	}

	void release(const Packer::Rectangle& rectangle)
	{
		if (rectangle.width <= 0 || rectangle.height <= 0)
			return;

		if (m_allocated > 0)
			m_allocated--;

		// Nothing left outstanding, so the atlas is empty by definition. Say so
		// exactly, instead of leaving behind whatever fragments coalescing couldn't
		// rejoin -- guillotine splits often meet along partial edges, which the merge
		// below cannot do anything with.
		if (m_allocated == 0)
		{
			clearFree();
			return;
		}

		m_free.push_back(rectangle);
		merge();
	}

	void reset()
	{
		clearFree();
		m_allocated = 0;
		m_epoch++;
	}

	uint32_t getEpoch() const
	{
		return m_epoch;
	}

private:
	AlignedVector< Packer::Rectangle > m_free;
	int32_t m_width;
	int32_t m_height;
	int32_t m_allocated = 0;
	uint32_t m_epoch = 0;

	void clearFree()
	{
		m_free.resize(0);
		m_free.push_back({ 0, 0, m_width, m_height });
	}

	/*! Coalesce free rectangles sharing a full edge.
	 *
	 * Without this the atlas fragment into slivers as cells are freed and re-allocated
	 * at slightly different sizes. Quadratic per pass and repeated until nothing more
	 * merge, which is fine for the few hundred rectangles an atlas hold.
	 */
	void merge()
	{
		bool merged = true;
		while (merged)
		{
			merged = false;
			for (int32_t i = 0; i < (int32_t)m_free.size() && !merged; ++i)
			{
				for (int32_t j = i + 1; j < (int32_t)m_free.size() && !merged; ++j)
				{
					Packer::Rectangle& a = m_free[i];
					const Packer::Rectangle& b = m_free[j];

					// Side by side, matching vertical extent.
					if (a.y == b.y && a.height == b.height)
					{
						if (a.x + a.width == b.x)
						{
							a.width += b.width;
							merged = true;
						}
						else if (b.x + b.width == a.x)
						{
							a.x = b.x;
							a.width += b.width;
							merged = true;
						}
					}

					// Stacked, matching horizontal extent.
					if (!merged && a.x == b.x && a.width == b.width)
					{
						if (a.y + a.height == b.y)
						{
							a.height += b.height;
							merged = true;
						}
						else if (b.y + b.height == a.y)
						{
							a.y = b.y;
							a.height += b.height;
							merged = true;
						}
					}

					if (merged)
						m_free.erase(m_free.begin() + j);
				}
			}
		}
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Packer", Packer, Object)

Packer::Packer(int32_t width, int32_t height)
{
	m_impl = new PackerImpl(width, height);
}

Packer::~Packer()
{
	delete m_impl;
}

bool Packer::insert(int32_t width, int32_t height, Rectangle& outRectangle)
{
	return m_impl->insert(width, height, outRectangle);
}

void Packer::release(const Rectangle& rectangle)
{
	m_impl->release(rectangle);
}

void Packer::reset()
{
	m_impl->reset();
}

uint32_t Packer::getEpoch() const
{
	return m_impl->getEpoch();
}

}
