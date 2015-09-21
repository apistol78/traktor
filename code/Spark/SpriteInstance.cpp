#include "Spark/IComponentInstance.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(const Sprite* sprite, const CharacterInstance* parent, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer)
:	CharacterInstance(parent)
,	m_sprite(sprite)
,	m_resourceManager(resourceManager)
,	m_soundPlayer(soundPlayer)
{
}

Ref< CharacterInstance > SpriteInstance::create(const std::wstring& id) const
{
	const Character* character = m_sprite->getCharacter(id);
	if (character)
		return character->createInstance(this, m_resourceManager, m_soundPlayer);
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

void SpriteInstance::getCharacters(RefArray< CharacterInstance >& outCharacters) const
{
	m_displayList.getCharacters(outCharacters);
}

void SpriteInstance::setComponent(const TypeInfo& componentType, IComponentInstance* component)
{
	m_components[&componentType] = component;
}

IComponentInstance* SpriteInstance::getComponent(const TypeInfo& componentType) const
{
	SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.find(&componentType);
	return i != m_components.end() ? i->second : 0;
}

Aabb2 SpriteInstance::getBounds() const
{
	Aabb2 bounds;

	if (m_shape)
		bounds = m_shape->getBounds();

	return bounds;
}

void SpriteInstance::update()
{
	// Update all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->update();

	// Update all characters visible in display list.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->update();
}

void SpriteInstance::render(render::RenderContext* renderContext) const
{
	if (!m_visible)
		return;

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
