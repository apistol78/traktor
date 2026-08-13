/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Ref.h"
#include "Spark/Packer.h"

namespace traktor::spark
{

/*! Cached rasterized sprite.
 * \ingroup Spark
 */
class AccCachedSprite : public RefCountImpl< IRefCount >
{
public:
	Ref< Packer > packer;
	uint32_t epoch = 0;
	uint32_t version = 0;
	Packer::Rectangle rect = { 0, 0, 0, 0 };
	Aabb2 bounds;
	float scale = 0.0f;
	bool valid = false;

	virtual ~AccCachedSprite()
	{
		if (packer != nullptr && packer->getEpoch() == epoch)
			packer->release(rect);
	}
};

}
