/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Cube texture.
 * \ingroup Render
 */
class T_DLLCLASS ICubeTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	enum Side
	{
		SdPositiveX	= 0,
		SdNegativeX = 1,
		SdPositiveY = 2,
		SdNegativeY = 3,
		SdPositiveZ = 4,
		SdNegativeZ = 5
	};

	/*! Size of cube map side in pixels.
	 */
	virtual int32_t getSide() const = 0;

	/*! Get number of mip maps.
	 *
	 * \return Number of mips.
	 */
	virtual int32_t getMips() const = 0;
	
	/*! Lock access to texture data.
	 *
	 * \param side Cube side.
	 * \param level Mip level.
	 * \param lock Information about locked region.
	 * \return True if locked.
	 */
	virtual bool lock(int32_t side, int32_t level, Lock& lock) = 0;

	/*! Unlock access to texture data.
	 *
	 * \param side Cube side.
	 * \param level Mip level.
	 */
	virtual void unlock(int32_t side, int32_t level) = 0;
};

}
