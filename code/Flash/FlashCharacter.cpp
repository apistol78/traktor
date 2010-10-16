#include "Flash/FlashCharacter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacter", FlashCharacter, ActionObject)

FlashCharacter::FlashCharacter(uint16_t id)
:	ActionObject("Object")
,	m_id(id)
{
}

uint16_t FlashCharacter::getId() const
{
	return m_id;
}

	}
}
