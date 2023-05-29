/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class IImageFilter;
class Image;

	}

	namespace shape
	{
	
class IblProbe : public Object
{
	T_RTTI_CLASS;

public:
	explicit IblProbe(const drawing::Image* radiance);

	Color4f sampleRadiance(const Vector4& direction) const;

	const drawing::Image* getRadianceImage() const { return m_radiance; }

private:
	Ref< const drawing::Image > m_radiance;
};
	
	}
}
