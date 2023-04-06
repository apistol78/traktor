/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Editor/IrradianceGridAsset.h"

namespace traktor::world
{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.IrradianceGridAsset", 2, IrradianceGridAsset, editor::Asset)

void IrradianceGridAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion< IrradianceGridAsset >() >= 1)
		s >> Member< float >(L"intensity", m_intensity, AttributeUnit(UnitType::Percent));

	if (s.getVersion< IrradianceGridAsset >() >= 2)
		s >> Member< bool >(L"cancelSun", m_cancelSun);
}

}
