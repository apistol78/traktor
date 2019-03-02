#include "Resource/IResourceManager.h"
#include "Spark/Context.h"
#include "Spark/Font.h"
#include "Spark/ICharacterBuilder.h"
#include "Spark/Text.h"
#include "Spark/TextData.h"
#include "Spark/TextFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextFactory", TextFactory, ICharacterFactory)

TypeInfoSet TextFactory::getCharacterTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextData >());
	return typeSet;
}

Ref< Character > TextFactory::create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const
{
	const TextData* textData = mandatory_non_null_type_cast< const TextData* >(characterData);

	resource::Proxy< Font > font;
	if (textData->m_font)
	{
		if (!context->getResourceManager()->bind(textData->m_font, font))
			return 0;
	}

	Ref< Text > instance = new Text(parent, font);
	instance->setTransform(textData->getTransform());
	instance->setText(textData->m_text);
	instance->setHeight(textData->m_height);
	instance->setBounds(Aabb2(-textData->m_origin, -textData->m_origin + textData->m_size));
	instance->setHorizontalAlign(textData->m_horizontalAlign);
	instance->setVerticalAlign(textData->m_verticalAlign);

	return instance;
}

	}
}
