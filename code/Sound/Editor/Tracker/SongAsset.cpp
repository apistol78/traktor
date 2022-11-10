/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Tracker/SongAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SongAsset", 0, SongAsset, ISerializable)

SongAsset::SongAsset()
:	m_presence(0.0f)
,	m_presenceRate(0.25f)
,	m_bpm(125)
{
}

void SongAsset::setBpm(int32_t bpm)
{
	m_bpm = bpm;
}

void SongAsset::addPattern(const PatternData* pattern)
{
	m_patterns.push_back(pattern);
}

void SongAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));
	s >> Member< float >(L"presence", m_presence, AttributeRange(0.0f));
	s >> Member< float >(L"presenceRate", m_presenceRate, AttributeRange(0.0f));
	s >> Member< int32_t >(L"bpm", m_bpm);
	s >> MemberRefArray< const PatternData >(L"patterns", m_patterns);
}

	}
}
