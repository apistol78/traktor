/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Compute/ComputeTexture.h"
#include "World/Entity/ComputeTextureComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComputeTextureComponent", ComputeTextureComponent, IWorldComponent)

void ComputeTextureComponent::add(render::ComputeTexture* texture)
{
	m_textures.push_back(texture);
}

void ComputeTextureComponent::remove(render::ComputeTexture* texture)
{
	m_textures.remove(texture);
}

void ComputeTextureComponent::destroy()
{
	m_textures.clear();
}

void ComputeTextureComponent::update(World* world, const UpdateParams& update)
{
}

}
