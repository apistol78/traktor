#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Sprite", 0, Sprite, Character)

Ref< CharacterInstance > Sprite::createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const
{
	Ref< SpriteInstance > instance = new SpriteInstance();

	if (m_shape)
	{
		if (!resourceManager->bind(m_shape, instance->m_shape))
			return 0;
	}

	instance->m_children.resize(m_children.size());
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		instance->m_children[i] = m_children[i]->createInstance(instance, resourceManager);
		if (!instance->m_children[i])
			return 0;
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
