/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Terrain/Editor/IFallOff.h"

namespace traktor::drawing
{

class Image;

}

namespace traktor::terrain
{

class ImageFallOff : public IFallOff
{
	T_RTTI_CLASS;

public:
	explicit ImageFallOff(const drawing::Image* image);

	virtual float evaluate(float x, float y) const override final;

private:
	Ref< const drawing::Image > m_image;
};

}
