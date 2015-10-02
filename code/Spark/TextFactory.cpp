#include "Resource/IResourceManager.h"
#include "Spark/Font.h"
#include "Spark/ICharacterBuilder.h"
#include "Spark/Text.h"
#include "Spark/TextFactory.h"
#include "Spark/TextInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextFactory", TextFactory, ICharacterFactory)

TextFactory::TextFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

TypeInfoSet TextFactory::getCharacterTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Text >());
	return typeSet;
}

Ref< CharacterInstance > TextFactory::create(const ICharacterBuilder* builder, const Character* character, const CharacterInstance* parent, const std::wstring& name) const
{
	const Text* text = mandatory_non_null_type_cast< const Text* >(character);

	resource::Proxy< Font > font;
	if (text->m_font)
	{
		if (!m_resourceManager->bind(text->m_font, font))
			return 0;
	}

	Ref< TextInstance > instance = new TextInstance(parent, font);
	instance->setTransform(text->getTransform());
	instance->setText(text->m_text);
	instance->setHeight(text->m_height);
	instance->setBounds(Aabb2(-text->m_origin, -text->m_origin + text->m_size));
	instance->setHorizontalAlign(text->m_horizontalAlign);
	instance->setVerticalAlign(text->m_verticalAlign);

	return instance;
}

	}
}
