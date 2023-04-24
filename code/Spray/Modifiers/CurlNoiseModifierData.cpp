/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/CurlNoiseModifier.h"
#include "Spray/Modifiers/CurlNoiseModifierData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.CurlNoiseModifierData", 0, CurlNoiseModifierData, ModifierData)

Ref< const Modifier > CurlNoiseModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	if (abs(m_factor) > FUZZY_EPSILON)
		return new CurlNoiseModifier(m_factor);
	else
		return nullptr;
}

void CurlNoiseModifierData::serialize(ISerializer& s)
{
	s >> Member< float >(L"factor", m_factor);
}

}
