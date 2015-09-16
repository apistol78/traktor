#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/Font.h"
#include "Spark/Text.h"
#include "Spark/TextInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Text", 0, Text, Character)

Text::Text()
:	m_height(0.0f)
{
}

Ref< CharacterInstance > Text::createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const
{
	Ref< TextInstance > instance = new TextInstance(parent);
	if (m_font)
	{
		if (!resourceManager->bind(m_font, instance->m_font))
			return 0;
	}
	instance->setText(m_text);
	instance->setHeight(m_height);
	return instance;
}

void Text::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> resource::Member< Font >(L"font", m_font);
	s >> Member< std::wstring >(L"text", m_text);
	s >> Member< float >(L"height", m_height);
}

	}
}
