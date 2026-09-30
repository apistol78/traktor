/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ColorCorrectionComponent.h"

#include "Render/ITexture.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ColorCorrectionComponent", ColorCorrectionComponent, IWorldComponent)

ColorCorrectionComponent::ColorCorrectionComponent(const resource::Proxy< render::ITexture >& colorGrading)
	: m_colorGrading(colorGrading)
{
}

void ColorCorrectionComponent::destroy()
{
	m_colorGrading.clear();
}

void ColorCorrectionComponent::update(World* world, const UpdateParams& update)
{
}

render::ITexture* ColorCorrectionComponent::getColorGrading() const
{
	return m_colorGrading;
}

}
