/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Sound", Sound, Object)

Sound::Sound(
	ISoundBuffer* buffer,
	handle_t category,
	float gain,
	float presence,
	float presenceRate,
	float range
)
:	m_buffer(buffer)
,	m_category(category)
,	m_gain(gain)
,	m_presence(presence)
,	m_presenceRate(presenceRate)
,	m_range(range)
{
}

	}
}
