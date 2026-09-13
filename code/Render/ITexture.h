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
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Texture base class.
 * \ingroup Render
 */
class T_DLLCLASS ITexture : public Object
{
	T_RTTI_CLASS;

public:
	struct Lock
	{
		size_t pitch;
		void* bits;
	};

	/*! Sub region of a texture level, in texels. */
	struct Region
	{
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	struct Size
	{
		int32_t x;
		int32_t y;
		int32_t z;
		int32_t mips;
	};

	enum Side : int32_t
	{
		PositiveX = 0,
		NegativeX = 1,
		PositiveY = 2,
		NegativeY = 3,
		PositiveZ = 4,
		NegativeZ = 5
	};

	/*! Release is fenced to ensure resource is alive during rendering.
	 *
	 * \sa ResourceMorgue.
	 */
	virtual void release(void* owner) const noexcept override final;

	/*! Destroy texture.
	 */
	virtual void destroy() = 0;

	/*! Get size of texture.
	 *
	 * \return Texture size in pixels.
	 */
	virtual Size getSize() const = 0;

	/*! Get bindless index of texture.
	 * 
	 * \return Bindless index, -1 if no index associated.
	 */
	virtual int32_t getBindlessIndex() const = 0;

	/*! Lock access to texture data.
	 *
	 * \param side Cube map side (0 if not cube map).
	 * \param level Mip level.
	 * \param lock Information about locked region.
	 * \return True if locked.
	 */
	virtual bool lock(int32_t side, int32_t level, Lock& lock) = 0;

	/*! Unlock access to texture data.
	 *
	 * \param side Cube map side (0 if not cube map).
	 * \param level Mip level.
	 */
	virtual void unlock(int32_t side, int32_t level) = 0;

	/*! Unlock access to texture data, transferring only a sub region.
	 *
	 * Only \a region is guaranteed to have been modified since the lock; a backend
	 * may use this to limit the amount of data transferred. The default implementation
	 * transfers the entire level.
	 *
	 * \param side Cube map side (0 if not cube map).
	 * \param level Mip level.
	 * \param region Region, in texels of the given level, modified since the lock.
	 */
	virtual void unlock(int32_t side, int32_t level, const Region& region);

	/*! Resolve render texture.
	 *
	 * \return Render texture.
	 */
	virtual ITexture* resolve() = 0;
};

}
