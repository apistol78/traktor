/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor::render
{

class IRenderSystem;
class ITexture;

}

namespace traktor::spark
{

class AccBitmapRect;
class FillStyle;

/*! Gradient cache for accelerated rendering.
 * \ingroup Spark
 */
class AccGradientCache : public Object
{
public:
	explicit AccGradientCache(render::IRenderSystem* renderSystem);

	virtual ~AccGradientCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getGradientTexture(const FillStyle& style);

	void synchronize();

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ITexture > m_gradientsTexture;
	SmallMap< uint64_t, Ref< AccBitmapRect > > m_cache;
	AutoArrayPtr< uint8_t > m_gradientsData;
	uint32_t m_currentGradientColumn = 0;
	uint32_t m_nextGradient = 0;
	bool m_dirty = false;
};

}
