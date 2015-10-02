#include "Spark/ICharacterBuilder.h"
#include "Spark/IComponentInstance.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(const ICharacterBuilder* builder, const Sprite* sprite, const CharacterInstance* parent)
:	CharacterInstance(parent)
,	m_builder(builder)
,	m_sprite(sprite)
,	m_mousePressed(false)
,	m_mouseInside(false)
{
}

Ref< CharacterInstance > SpriteInstance::create(const std::wstring& id) const
{
	const Character* character = m_sprite->getCharacter(id);
	if (character)
		return m_builder->create(character, this, id);
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

void SpriteInstance::eventMouseDown(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Check if mouse being pressed inside this sprite.
	if (!m_components.empty() && getBounds().inside(localPosition))
	{
		T_ASSERT (!m_mousePressed);

		// Propagate event to all components.
		for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
			i->second->eventMousePress(localPosition, button);

		m_mousePressed = true;
	}

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseDown(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseDown(localPosition, button);
}

void SpriteInstance::eventMouseUp(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseUp(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseUp(localPosition, button);

	// Also issue mouse release events if mouse was pressed inside this sprite.
	if (m_mousePressed)
	{
		// Propagate event to all components.
		for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
			i->second->eventMouseRelease(localPosition, button);

		m_mousePressed = false;
	}
}

void SpriteInstance::eventMouseMove(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseMove(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseMove(localPosition, button);

	// Check if mouse is entering or leaving this sprite.
	if (!m_components.empty())
	{
		bool inside = getBounds().inside(localPosition);
		if (m_mouseInside != inside)
		{
			// Propagate event to all components.
			if (inside)
			{
				for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
					i->second->eventMouseEnter(localPosition, button);
			}
			else
			{
				for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
					i->second->eventMouseLeave(localPosition, button);
			}
			m_mouseInside = inside;
		}
	}
}

void SpriteInstance::eventMouseWheel(const Vector2& position, int32_t delta)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponentInstance > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseWheel(localPosition, delta);

	// Propagate event to all visible characters.
	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseWheel(localPosition, delta);
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
	Aabb2 bounds;

	if (m_shape)
		bounds = m_shape->getBounds();

	RefArray< CharacterInstance > characters;
	m_displayList.getCharacters(characters);

	for (RefArray< CharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
	{
		Vector2 childExtents[4];
		(*i)->getBounds().getExtents(childExtents);
		
		Matrix33 childTransform = (*i)->getTransform();
		bounds.contain(childTransform * childExtents[0]);
		bounds.contain(childTransform * childExtents[1]);
		bounds.contain(childTransform * childExtents[2]);
		bounds.contain(childTransform * childExtents[3]);
	}

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
		m_shape->render(renderContext, getFullTransform(), m_colorTransform);
}

	}
}
