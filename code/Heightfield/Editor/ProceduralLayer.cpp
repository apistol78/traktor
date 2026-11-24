/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ProceduralLayer.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Heightfield/Editor/ProceduralBrush.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ProceduralLayer", ProceduralLayer, ISerializable)

void ProceduralLayer::serialize(ISerializer& s)
{
	s >> MemberRef< ProceduralBrush >(L"brush", m_brush);
}

}
