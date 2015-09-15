#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
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
	std::map< std::wstring, Ref< Character > >::const_iterator i = m_characters.find(id);
	return i != m_characters.end() ? i->second : 0;
}

Ref< CharacterInstance > Sprite::createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const
{
	Ref< SpriteInstance > instance = new SpriteInstance(this, parent, resourceManager);

	if (m_shape)
	{
		if (!resourceManager->bind(m_shape, instance->m_shape))
			return 0;
	}

	for (size_t i = 0; i < m_components.size(); ++i)
	{
		Ref< IComponentInstance > componentInstance = m_components[i]->createInstance(instance, resourceManager);
		if (!componentInstance)
			return 0;

		instance->addComponent(componentInstance);
	}

	return instance;
}

void Sprite::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> resource::Member< Shape >(L"shape", m_shape);
	s >> MemberStlMap< std::wstring, Ref< Character >, MemberStlPair< std::wstring, Ref< Character >, Member< std::wstring >, MemberRef< Character > > >(L"characters", m_characters);
}

	}
}
