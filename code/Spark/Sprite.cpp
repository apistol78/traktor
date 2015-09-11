#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
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

Ref< CharacterInstance > Sprite::createInstance(StageInstance* stage, const CharacterInstance* parent, resource::IResourceManager* resourceManager) const
{
	Ref< SpriteInstance > instance = new SpriteInstance();

	if (m_shape)
	{
		if (!resourceManager->bind(m_shape, instance->m_shape))
			return 0;
	}

	for (size_t i = 0; i < m_children.size(); ++i)
	{
		Ref< CharacterInstance > childInstance = m_children[i]->createInstance(stage, instance, resourceManager);
		if (!childInstance)
			return 0;

		instance->addChild(childInstance);
	}

	for (size_t i = 0; i < m_components.size(); ++i)
	{
		Ref< IComponentInstance > componentInstance = m_components[i]->createInstance(stage, instance, resourceManager);
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
	s >> MemberRefArray< Character >(L"children", m_children);
}

	}
}
