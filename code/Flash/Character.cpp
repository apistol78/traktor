/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/Character.h"
#include "Flash/Types.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Character", Character, ISerializable)

Character::Character()
:	m_id(0)
,	m_tag(allocateCacheTag())
{
}

Character::Character(uint16_t id)
:	m_id(id)
,	m_tag(allocateCacheTag())
{
}

void Character::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"id", m_id);
}

	}
}
