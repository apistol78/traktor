#include "Flash/FlashCharacter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacter", FlashCharacter, Object)

FlashCharacter::FlashCharacter(uint16_t id)
:	m_id(id)
{
}

uint16_t FlashCharacter::getId() const
{
	return m_id;
}

	}
}
