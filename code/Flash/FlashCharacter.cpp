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

uint16_t FlashCharacter::getId() const
{
	return m_id;
}

int32_t FlashCharacter::getCacheTag() const
{
	return m_tag;
}

void FlashCharacter::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"id", m_id);
}

	}
}
