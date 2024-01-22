/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/VortexModifier.h"
#include "Spray/Modifiers/VortexModifierData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.VortexModifierData", 0, VortexModifierData, ModifierData)

Ref< const Modifier > VortexModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new VortexModifier(
		m_axis,
		m_tangentForce,
		m_normalConstantForce,
		m_normalDistance,
		m_normalDistanceForce,
		m_world
	);
}

void VortexModifierData::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"axis", m_axis, AttributeDirection());
	s >> Member< float >(L"tangentForce", m_tangentForce);
	s >> Member< float >(L"normalConstantForce", m_normalConstantForce);
	s >> Member< float >(L"normalDistance", m_normalDistance);
	s >> Member< float >(L"normalDistanceForce", m_normalDistanceForce);
	s >> Member< bool >(L"world", m_world);
}

}
