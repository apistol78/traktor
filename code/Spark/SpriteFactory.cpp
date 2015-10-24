#include "Resource/IResourceManager.h"
#include "Spark/Context.h"
#include "Spark/ICharacterBuilder.h"
#include "Spark/IComponentData.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteData.h"
#include "Spark/SpriteFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteFactory", SpriteFactory, ICharacterFactory)

SpriteFactory::SpriteFactory(bool createComponents)
:	m_createComponents(createComponents)
{
}

TypeInfoSet SpriteFactory::getCharacterTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SpriteData >());
	return typeSet;
}

Ref< Character > SpriteFactory::create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const
{
	const SpriteData* spriteData = mandatory_non_null_type_cast< const SpriteData* >(characterData);

	// Create sprite instance.
	Ref< Sprite > sprite = new Sprite(context, builder, spriteData, parent);
	sprite->setTransform(spriteData->getTransform());

	// Create shape.
	if (spriteData->m_shape)
	{
		if (!context->getResourceManager()->bind(spriteData->m_shape, sprite->m_shape))
			return 0;
	}

	// Create child characters.
	int32_t depth = -100000;
	for (AlignedVector< SpriteData::NamedCharacter >::const_iterator i = spriteData->m_frame.begin(); i != spriteData->m_frame.end(); ++i)
	{
		if (!i->character)
			continue;

		Ref< Character > child = builder->create(context, i->character, sprite, i->name);
		if (!child)
			return 0;

		child->setName(i->name);

		sprite->place(depth, child);
		++depth;
	}

	// Create components.
	if (m_createComponents)
	{
		for (size_t i = 0; i < spriteData->m_components.size(); ++i)
		{
			if (!spriteData->m_components[i])
				continue;

			Ref< IComponent > component = spriteData->m_components[i]->createInstance(context, sprite);
			if (!component)
				return 0;

			sprite->setComponent(component);
		}
	}

	return sprite;
}

	}
}
