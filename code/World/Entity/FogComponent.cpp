/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/FogComponent.h"

#include "World/Entity/FogComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.FogComponent", FogComponent, IWorldComponent)

FogComponent::FogComponent(const FogComponentData* data, const resource::Proxy< render::Shader >& mediumShader)
{
	m_mediumColor = data->m_mediumColor;
	m_mediumDensity = data->m_mediumDensity;
	m_mediumShader = mediumShader;

	m_distanceFogEnable = data->m_distanceFogEnable;
	m_fogDistance = data->m_fogDistance;
	m_fogElevation = data->m_fogElevation;

	m_volumetricFogEnable = data->m_volumetricFogEnable;
	m_maxDistance = data->m_maxDistance;

	m_phaseForward = data->m_phaseForward;
	m_phaseBackward = data->m_phaseBackward;
	m_phaseBlend = data->m_phaseBlend;
}

void FogComponent::destroy()
{
	m_mediumShader.clear();
}

void FogComponent::update(World* world, const UpdateParams& update)
{
}

}
