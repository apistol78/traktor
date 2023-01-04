/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor::spray
{

/*! Emitter vertex.
 * \ingroup Spray
 */
#pragma pack(1)
struct EmitterVertex
{
	float extent[4];
};
#pragma pack()

#pragma pack(1)
struct EmitterPoint
{
	float positionAndOrientation[4];
	float velocityAndRandom[4];
	float alphaAndSize[4];
	float colorAndAge[4];
};
#pragma pack()

/*! Ribbon trail vertex.
 * \ingroup Spray
 */
#pragma pack(1)
struct TrailVertex
{
	float position[4];
	float direction[4];
	float uv[4];
};
#pragma pack()

}
