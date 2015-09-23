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

void SpriteInstance::setAlpha(float alpha)
{
	m_colorTransform.alpha[0] = alpha;
}

float SpriteInstance::getAlpha() const
{
	return m_colorTransform.alpha[0];
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

void SpriteInstance::eventKey(wchar_t unicode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKey(unicode);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKey(unicode);
}

void SpriteInstance::eventKeyDown(int32_t keyCode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKeyDown(keyCode);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKeyDown(keyCode);
}

void SpriteInstance::eventKeyUp(int32_t keyCode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKeyUp(keyCode);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKeyUp(keyCode);
}

void SpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseDown(x, y, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseDown(x, y, button);
}

void SpriteInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseUp(x, y, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseUp(x, y, button);
}

void SpriteInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseMove(x, y, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseMove(x, y, button);
}

void SpriteInstance::eventMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseWheel(x, y, delta);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseWheel(x, y, delta);
}

void SpriteInstance::eventViewResize(int32_t width, int32_t height)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventViewResize(width, height);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventViewResize(width, height);
}

Aabb2 SpriteInstance::getBounds() const
{
	return m_shape ? m_shape->getBounds() : Aabb2();
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
		m_shape->render(renderContext, getFullTransform(), m_colorTransform);
}

	}
}
