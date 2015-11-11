#include "Spark/ICharacterBuilder.h"
#include "Spark/IComponent.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteData.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Sprite", Sprite, Character)

Sprite::Sprite(const Context* context, const ICharacterBuilder* builder, const SpriteData* spriteData, const Character* parent)
:	Character(parent)
,	m_context(context)
,	m_builder(builder)
,	m_spriteData(spriteData)
,	m_mousePressed(false)
,	m_mouseInside(false)
{
}

const Context* Sprite::getContext() const
{
	return m_context;
}

void Sprite::setShape(Shape* shape)
{
	m_shape = resource::Proxy< Shape >(shape);
}

const resource::Proxy< Shape >& Sprite::getShape() const
{
	return m_shape;
}

Ref< Character > Sprite::create(const std::wstring& id) const
{
	const CharacterData* characterData = m_spriteData->getCharacter(id);
	if (characterData)
		return m_builder->create(m_context, characterData, this, id);
	else
		return 0;
}

void Sprite::place(int32_t depth, Character* instance)
{
	m_displayList.place(depth, instance);
}

void Sprite::remove(int32_t depth)
{
	m_displayList.remove(depth);
}

void Sprite::remove(Character* instance)
{
	m_displayList.remove(instance);
}

void Sprite::setAlpha(float alpha)
{
	m_colorTransform.alpha[0] = alpha;
}

float Sprite::getAlpha() const
{
	return m_colorTransform.alpha[0];
}

void Sprite::getCharacters(RefArray< Character >& outCharacters) const
{
	m_displayList.getCharacters(outCharacters);
}

void Sprite::setComponent(IComponent* component)
{
	m_components[&type_of(component)] = component;
}

IComponent* Sprite::getComponent(const TypeInfo& componentType) const
{
	SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.find(&componentType);
	return i != m_components.end() ? i->second : 0;
}

void Sprite::eventKey(wchar_t unicode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKey(unicode);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKey(unicode);
}

void Sprite::eventKeyDown(int32_t keyCode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKeyDown(keyCode);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKeyDown(keyCode);
}

void Sprite::eventKeyUp(int32_t keyCode)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventKeyUp(keyCode);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventKeyUp(keyCode);
}

void Sprite::eventMouseDown(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Check if mouse being pressed inside this sprite.
	if (!m_components.empty() && getBounds().inside(localPosition))
	{
		T_ASSERT (!m_mousePressed);

		// Propagate event to all components.
		for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
			i->second->eventMousePress(localPosition, button);

		m_mousePressed = true;
	}

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseDown(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseDown(localPosition, button);
}

void Sprite::eventMouseUp(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseUp(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseUp(localPosition, button);

	// Also issue mouse release events if mouse was pressed inside this sprite.
	if (m_mousePressed)
	{
		// Propagate event to all components.
		for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
			i->second->eventMouseRelease(localPosition, button);

		m_mousePressed = false;
	}
}

void Sprite::eventMouseMove(const Vector2& position, int32_t button)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseMove(localPosition, button);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
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
				for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
					i->second->eventMouseEnter(localPosition, button);
			}
			else
			{
				for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
					i->second->eventMouseLeave(localPosition, button);
			}
			m_mouseInside = inside;
		}
	}
}

void Sprite::eventMouseWheel(const Vector2& position, int32_t delta)
{
	Vector2 localPosition = getTransform().inverse() * position;

	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventMouseWheel(localPosition, delta);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventMouseWheel(localPosition, delta);
}

void Sprite::eventViewResize(int32_t width, int32_t height)
{
	// Propagate event to all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->eventViewResize(width, height);

	// Propagate event to all visible characters.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->eventViewResize(width, height);
}

Aabb2 Sprite::getBounds() const
{
	Aabb2 bounds = m_spriteData->getBounds();

	if (m_shape)
		bounds = m_shape->getBounds();

	RefArray< Character > characters;
	m_displayList.getCharacters(characters);

	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
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

void Sprite::update()
{
	// Update all components.
	for (SmallMap< const TypeInfo*, Ref< IComponent > >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		i->second->update();

	// Update all characters visible in display list.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->update();
}

void Sprite::render(render::RenderContext* renderContext) const
{
	if (!m_visible)
		return;

	// Render all child characters visible in display list first.
	RefArray< Character > characters;
	m_displayList.getCharacters(characters);
	for (RefArray< Character >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		(*i)->render(renderContext);

	// Render this sprite's shape.
	if (m_shape)
		m_shape->render(renderContext, getFullTransform(), m_colorTransform);
}

	}
}
