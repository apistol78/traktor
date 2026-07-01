/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/ITexture.h"
#include "Resource/Id.h"
#include "World/IWorldComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Compute texture component data.
 * \ingroup World
 *
 * Serializable data for ComputeTextureComponent.
 * References one or more compute texture resources
 * to be dispatched each frame.
 */
class T_DLLCLASS ComputeTextureComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const AlignedVector< resource::Id< render::ITexture > >& getTextures() const { return m_textures; }

private:
	AlignedVector< resource::Id< render::ITexture > > m_textures;
};

}
