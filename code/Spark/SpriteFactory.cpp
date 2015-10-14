#include "Resource/IResourceManager.h"
#include "Spark/ICharacterBuilder.h"
#include "Spark/IComponent.h"
#include "Spark/Shape.h"
#include "Spark/ShapeRenderable.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteFactory.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteFactory", SpriteFactory, ICharacterFactory)

SpriteFactory::SpriteFactory(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer, bool createComponents)
:	m_resourceManager(resourceManager)
,	m_soundPlayer(soundPlayer)
,	m_createComponents(createComponents)
{
}

TypeInfoSet SpriteFactory::getCharacterTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Sprite >());
	return typeSet;
}

Ref< CharacterInstance > SpriteFactory::create(const ICharacterBuilder* builder, const Character* character, const CharacterInstance* parent, const std::wstring& name) const
{
	const Sprite* sprite = mandatory_non_null_type_cast< const Sprite* >(character);

	// Create sprite instance.
	Ref< SpriteInstance > instance = new SpriteInstance(builder, sprite, parent);
	instance->setTransform(sprite->getTransform());

	// Create shape.
	if (sprite->m_shape)
	{
		resource::Proxy< Shape > shape;
		if (!m_resourceManager->bind(sprite->m_shape, shape))
			return 0;
		instance->m_renderable = new ShapeRenderable(shape);
	}

	// Create child characters.
	int32_t depth = -100000;
	for (AlignedVector< Sprite::NamedCharacter >::const_iterator i = sprite->m_frame.begin(); i != sprite->m_frame.end(); ++i)
	{
		if (!i->character)
			continue;

		Ref< CharacterInstance > childInstance = builder->create(i->character, instance, i->name);
		if (!childInstance)
			return 0;

		childInstance->setName(i->name);

		instance->place(depth, childInstance);
		++depth;
	}

	// Create components.
	if (m_createComponents)
	{
		for (size_t i = 0; i < sprite->m_components.size(); ++i)
		{
			if (!sprite->m_components[i])
				continue;

			Ref< IComponentInstance > componentInstance = sprite->m_components[i]->createInstance(instance, m_resourceManager, m_soundPlayer);
			if (!componentInstance)
				return 0;

			instance->setComponent(type_of(sprite->m_components[i]), componentInstance);
		}
	}

	return instance;
}

	}
}
