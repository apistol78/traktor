#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/IComponent.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Sprite", 0, Sprite, Character)

const Character* Sprite::getCharacter(const std::wstring& id) const
{
	SmallMap< std::wstring, Ref< Character > >::const_iterator i = m_characters.find(id);
	return i != m_characters.end() ? i->second : 0;
}

Ref< CharacterInstance > Sprite::createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const
{
	Ref< SpriteInstance > instance = new SpriteInstance(this, parent, resourceManager, soundPlayer);

	// Create this sprite's shape.
	if (m_shape)
	{
		if (!resourceManager->bind(m_shape, instance->m_shape))
			return 0;
	}

	// Create child characters.
	int32_t depth = -100000;
	for (AlignedVector< Place >::const_iterator i = m_place.begin(); i != m_place.end(); ++i)
	{
		if (!i->character)
			continue;

		Ref< CharacterInstance > placeInstance = i->character->createInstance(instance, resourceManager, soundPlayer);
		if (!placeInstance)
			return 0;

		placeInstance->setName(i->name);
		placeInstance->setTransform(i->transform);

		instance->place(depth, placeInstance);
		++depth;
	}

	// Create components.
	for (size_t i = 0; i < m_components.size(); ++i)
	{
		if (!m_components[i])
			continue;

		Ref< IComponentInstance > componentInstance = m_components[i]->createInstance(instance, resourceManager, soundPlayer);
		if (!componentInstance)
			return 0;

		instance->setComponent(type_of(m_components[i]), componentInstance);
	}

	return instance;
}

void Sprite::serialize(ISerializer& s)
{
	s >> MemberRefArray< IComponent >(L"components", m_components);
	s >> resource::Member< Shape >(L"shape", m_shape);
	s >> MemberSmallMap< std::wstring, Ref< Character >, Member< std::wstring >, MemberRef< Character > >(L"characters", m_characters);
	s >> MemberAlignedVector< Place, MemberComposite< Place > >(L"place", m_place);
}

Sprite::Place::Place()
:	transform(Matrix33::identity())
{
}

void Sprite::Place::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> MemberRef< Character >(L"character", character);
	s >> Member< Matrix33 >(L"transform", transform);
}

	}
}
