/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ColorCorrectionComponentData.h"

#include "Core/Serialization/ISerializer.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ColorCorrectionComponentData", 0, ColorCorrectionComponentData, IWorldComponentData)

void ColorCorrectionComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ITexture >(L"colorGrading", m_colorGrading);
}

}
