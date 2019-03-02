#include "Flash/TextInstance.h"
#include "Flash/Text.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.TextInstance", TextInstance, CharacterInstance)

TextInstance::TextInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Text* text)
:	CharacterInstance(context, "TextField", dictionary, parent)
,	m_text(text)
{
}

const Text* TextInstance::getText() const
{
	return m_text;
}

Aabb2 TextInstance::getBounds() const
{
	return getTransform() * m_text->getTextBounds();
}

	}
}
