/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

/*! Rectangle allocator for atlases.
 * \ingroup Spark
 *
 * Guillotine allocator; keeps a list of free rectangles, splits one to satisfy each
 * allocation, and coalesces neighbours when a rectangle is handed back. Unlike a
 * skyline packer this can genuinely re-use freed space, which matters for atlases
 * whose contents come and go while the atlas itself lives on.
 */
class T_DLLCLASS Packer : public Object
{
	T_RTTI_CLASS;

public:
	struct Rectangle
	{
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	explicit Packer(int32_t width, int32_t height);

	virtual ~Packer();

	/*! Allocate a rectangle.
	 *
	 * \return False if it doesn't fit in the space remaining.
	 */
	bool insert(int32_t width, int32_t height, Rectangle& outRectangle);

	/*! Release a rectangle.
	 *
	 * Must be exactly a rectangle returned by insert, and handed back only once;
	 * anything else corrupt the free list into overlapping allocations.
	 */
	void release(const Rectangle& rectangle);

	/*! Discard every allocation.
	 *
	 * Bumps the generation so rectangles handed out beforehand can be recognized as
	 * stale. Those must not be released; the reset already reclaimed them.
	 */
	void reset();

	/*! Get generation; incremented by each reset. */
	uint32_t getEpoch() const;

private:
	class PackerImpl* m_impl;
};

}
