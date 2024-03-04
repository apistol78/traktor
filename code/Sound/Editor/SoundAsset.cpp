/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Types.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 8, SoundAsset, editor::Asset)

void SoundAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 2)
		s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));

	s >> Member< bool >(L"stream", m_stream);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preload", m_preload);

	if (s.getVersion() >= 7)
		s >> Member< bool >(L"compressed", m_compressed);

	if (s.getVersion() < 8)
	{
		if (s.getVersion() >= 3)
			s >> ObsoleteMember< float >(L"presence");

		if (s.getVersion() >= 4)
			s >> ObsoleteMember< float >(L"presenceRate");
	}

	if (s.getVersion() >= 5)
	{
		if (s.getVersion() >= 6)
			s >> Member< float >(L"gain", m_gain, AttributeUnit(UnitType::Decibel));
		else
		{
			float volumeLin = 1.0f;
			s >> Member< float >(L"volume", volumeLin);
			m_gain = linearToDecibel(volumeLin);
		}
	}
}

	}
}
