#include "Flash/FlashTextInstance.h"
#include "Flash/FlashText.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashTextInstance", FlashTextInstance, FlashCharacterInstance)

FlashTextInstance::FlashTextInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashText* text)
:	FlashCharacterInstance(context, "TextField", parent)
,	m_text(text)
{
}

const FlashText* FlashTextInstance::getText() const
{
	return m_text;
}

Aabb2 FlashTextInstance::getBounds() const
{
	Aabb2 textBounds = m_text->getTextBounds();
	textBounds.mn = getTransform() * textBounds.mn;
	textBounds.mx = getTransform() * textBounds.mx;
	return textBounds;
}

	}
}
