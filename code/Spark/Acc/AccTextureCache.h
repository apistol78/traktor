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
#include "Core/RefArray.h"

namespace traktor::render
{

class IRenderSystem;
class ITexture;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spark
{

class AccBitmapRect;
class Bitmap;

/*! Texture cache for accelerated rendering.
 * \ingroup Spark
 */
class AccTextureCache : public Object
{
public:
	explicit AccTextureCache(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		bool reuseTextures
	);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getBitmapTexture(const Bitmap& bitmap);

	void freeTexture(render::ITexture* texture);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::ITexture > m_freeTextures;
	bool m_reuseTextures;
};

}
