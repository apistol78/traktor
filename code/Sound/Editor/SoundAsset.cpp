/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeDecibel.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Types.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 6, SoundAsset, editor::Asset)

SoundAsset::SoundAsset()
:	m_stream(false)
,	m_preload(true)
,	m_presence(0.0f)
,	m_presenceRate(0.25f)
,	m_gain(0.0f)
{
}

void SoundAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion< SoundAsset >() >= 2)
		s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));

	s >> Member< bool >(L"stream", m_stream);
	
	if (s.getVersion< SoundAsset >() >= 1)
		s >> Member< bool >(L"preload", m_preload);

	if (s.getVersion< SoundAsset >() >= 3)
		s >> Member< float >(L"presence", m_presence, AttributeRange(0.0f));

	if (s.getVersion< SoundAsset >() >= 4)
		s >> Member< float >(L"presenceRate", m_presenceRate, AttributeRange(0.0f));

	if (s.getVersion< SoundAsset >() >= 5)
	{
		if (s.getVersion< SoundAsset >() >= 6)
			s >> Member< float >(L"gain", m_gain, AttributeDecibel());
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
