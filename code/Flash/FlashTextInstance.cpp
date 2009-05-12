#include "Flash/FlashTextInstance.h"
#include "Flash/FlashText.h"
#include "Flash/Action/Classes/AsTextField.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashTextInstance", FlashTextInstance, FlashCharacterInstance)

FlashTextInstance::FlashTextInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashText* text)
:	FlashCharacterInstance(context, AsTextField::getInstance(), parent)
,	m_text(text)
{
}

const FlashText* FlashTextInstance::getText() const
{
	return m_text;
}

SwfRect FlashTextInstance::getBounds() const
{
	SwfRect textBounds = m_text->getTextBounds();
	textBounds.min = getTransform() * textBounds.min;
	textBounds.max = getTransform() * textBounds.max;
	return textBounds;
}

	}
}
