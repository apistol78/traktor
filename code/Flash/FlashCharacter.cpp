#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashCharacter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacter", FlashCharacter, ISerializable)

FlashCharacter::FlashCharacter()
:	m_id(0)
{
}

FlashCharacter::FlashCharacter(uint16_t id)
:	m_id(id)
{
}

uint16_t FlashCharacter::getId() const
{
	return m_id;
}

bool FlashCharacter::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"id", m_id);
	return true;
}

	}
}
