/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DisplacementWorldComponentData.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Shader.h"
#include "World/Entity/DisplacementWorldComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DisplacementWorldComponentData", 0, DisplacementWorldComponentData, IWorldComponentData)

Ref< DisplacementWorldComponent > DisplacementWorldComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	Ref< DisplacementWorldComponent > component = new DisplacementWorldComponent();
	if (component->create(resourceManager, renderSystem, *this))
		return component;
	else
		return nullptr;
}

void DisplacementWorldComponentData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"resolution", m_resolution, AttributeRange(16, 4096));
	s >> Member< float >(L"extent", m_extent, AttributeRange(1.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"fadeRate", m_fadeRate, AttributeRange(0.0f));
}

}
