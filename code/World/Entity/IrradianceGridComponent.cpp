/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/IrradianceGridComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.IrradianceGridComponent", IrradianceGridComponent, IWorldComponent)

void IrradianceGridComponent::destroy()
{
	m_irradianceGrid = nullptr;
}

void IrradianceGridComponent::update(World* world, const UpdateParams& update)
{
}

void IrradianceGridComponent::setIrradianceGrid(const IrradianceGrid* irradianceGrid)
{
	m_irradianceGrid = irradianceGrid;
}

const IrradianceGrid* IrradianceGridComponent::getIrradianceGrid() const
{
	return m_irradianceGrid;
}

}
