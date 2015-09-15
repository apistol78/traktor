#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(const Sprite* sprite, const CharacterInstance* parent, resource::IResourceManager* resourceManager)
:	CharacterInstance(parent)
,	m_sprite(sprite)
,	m_resourceManager(resourceManager)
{
}

Ref< CharacterInstance > SpriteInstance::create(const std::wstring& id) const
{
	const Character* character = m_sprite->getCharacter(id);
	if (character)
		return character->createInstance(this, m_resourceManager);
	else
		return 0;
}

void SpriteInstance::place(int32_t depth, CharacterInstance* instance)
{
	m_displayList.place(depth, instance);
}

void SpriteInstance::remove(int32_t depth)
{
	m_displayList.remove(depth);
}

void SpriteInstance::update()
{
	CharacterInstance::update();

	// Update all characters visible in display list.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->update();
}

void SpriteInstance::render(render::RenderContext* renderContext) const
{
	// Render all child characters visible in display list first.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->render(renderContext);

	// Render this sprite's shape.
	if (m_shape)
		m_shape->render(renderContext, getFullTransform());
}

	}
}
