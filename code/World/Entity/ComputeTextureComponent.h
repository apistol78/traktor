/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ComputeTexture;

}

namespace traktor::world
{

/*! Compute texture world component.
 * \ingroup World
 *
 * Holds all compute textures that need to be dispatched
 * as part of the render graph each frame.
 */
class T_DLLCLASS ComputeTextureComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	void add(render::ComputeTexture* texture);

	void remove(render::ComputeTexture* texture);

	const RefArray< render::ComputeTexture >& getTextures() const { return m_textures; }

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

private:
	RefArray< render::ComputeTexture > m_textures;
};

}
