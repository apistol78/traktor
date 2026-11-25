/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/HeightfieldAssetLayer.h"

#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldAssetLayer", HeightfieldAssetLayer, ISerializable)

bool HeightfieldAssetLayer::isGlobal() const
{
	return false;
}

bool HeightfieldAssetLayer::haveData() const
{
	return false;
}

bool HeightfieldAssetLayer::haveMask() const
{
	return false;
}

void HeightfieldAssetLayer::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"id", m_id, AttributePrivate());
	s >> Member< std::wstring >(L"name", m_name);
}

}
