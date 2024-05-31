/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"

namespace traktor::render
{

class ITexture;

}

namespace traktor::spark
{

/*!
 * \ingroup Spark
 */
class AccBitmapRect : public RefCountImpl< IRefCount >
{
public:
	resource::Proxy< render::ITexture > texture;
	float rect[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	AccBitmapRect() = default;

	explicit AccBitmapRect(
		const resource::Proxy< render::ITexture >& texture_,
		float left,
		float top,
		float width,
		float height
	)
	{
		texture = texture_;
		rect[0] = left;
		rect[1] = top;
		rect[2] = width;
		rect[3] = height;
	}

	bool operator == (const AccBitmapRect& rh) const
	{
		if (texture != rh.texture)
			return false;

		return
			rect[0] == rh.rect[0] &&
			rect[1] == rh.rect[1] &&
			rect[2] == rh.rect[2] &&
			rect[3] == rh.rect[3];
	}

	bool operator != (const AccBitmapRect& rh) const
	{
		return !(*this == rh);
	}
};

}
