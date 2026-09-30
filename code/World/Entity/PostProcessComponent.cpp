/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/PostProcessComponent.h"

#include "Render/Image2/ImageGraph.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessComponent", PostProcessComponent, IWorldComponent)

PostProcessComponent::PostProcessComponent(const resource::Proxy< render::ImageGraph >& imageGraph)
	: m_imageGraph(imageGraph)
{
}

void PostProcessComponent::destroy()
{
	m_imageGraph.clear();
}

void PostProcessComponent::update(World* world, const UpdateParams& update)
{
}

render::ImageGraph* PostProcessComponent::getImageGraph() const
{
	return m_imageGraph;
}

}
