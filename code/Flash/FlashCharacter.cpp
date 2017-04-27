/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashTypes.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacter", FlashCharacter, ISerializable)

FlashCharacter::FlashCharacter()
:	m_id(0)
,	m_tag(allocateCacheTag())
{
}

FlashCharacter::FlashCharacter(uint16_t id)
:	m_id(id)
,	m_tag(allocateCacheTag())
{
}

void FlashCharacter::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"id", m_id);
}

	}
}
