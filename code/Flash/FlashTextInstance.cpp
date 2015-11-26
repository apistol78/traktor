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
	return getTransform() * m_text->getTextBounds();
}

	}
}
