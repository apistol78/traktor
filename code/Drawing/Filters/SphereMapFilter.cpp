/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/CubeMap.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/SphereMapFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SphereMapFilter", SphereMapFilter, IImageFilter)

void SphereMapFilter::apply(Image* image) const
{
	Ref< CubeMap > cubeMap = CubeMap::createFromImage(image);
	if (!cubeMap)
		return;

	Ref< Image > outputImage = cubeMap->createEquirectangular();
	if (!outputImage)
		return;

	image->swap(outputImage);
}

}
