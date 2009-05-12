#include "Flash/FlashCharacter.h"
#include "Flash/Action/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashCharacter", FlashCharacter, ActionObject)

FlashCharacter::FlashCharacter(uint16_t id)
:	ActionObject(AsObject::getInstance())
,	m_id(id)
{
}

uint16_t FlashCharacter::getId() const
{
	return m_id;
}

	}
}
