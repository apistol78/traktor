#include "Spark/Character.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Sprite.h"
#include "Spark/External.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/IGizmo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterAdapter", CharacterAdapter, Object)

CharacterAdapter::CharacterAdapter()
:	m_parent(0)
,	m_selected(false)
{
}

const std::wstring& CharacterAdapter::getName() const
{
	return m_name;
}

void CharacterAdapter::setTransform(const Matrix33& transform)
{
	m_character->setTransform(transform);
	m_characterInstance->setTransform(transform);
}

const Matrix33& CharacterAdapter::getTransform() const
{
	return m_characterInstance->getTransform();
}

Character* CharacterAdapter::getCharacter()
{
	return m_character;
}

CharacterInstance* CharacterAdapter::getCharacterInstance()
{
	return m_characterInstance;
}

CharacterAdapter* CharacterAdapter::getParent()
{
	return m_parent;
}

const RefArray< CharacterAdapter >& CharacterAdapter::getChildren()
{
	return m_children;
}

void CharacterAdapter::unlink()
{
	if (!m_parent || !is_a< Sprite* >(m_parent->getCharacter()))
		return;

	mandatory_non_null_type_cast< Sprite* >(m_parent->getCharacter())->remove(m_character);

	m_characterInstance = 0;
	m_character = 0;

	m_parent->m_children.remove(this);
	m_parent = 0;
}

void CharacterAdapter::attachGizmo(IGizmo* gizmo)
{
	if ((m_gizmo = gizmo) != 0)
		m_gizmo->attach(this);
}

void CharacterAdapter::detachGizmo()
{
	if (m_gizmo != 0)
	{
		m_gizmo->detach(this);
		m_gizmo = 0;
	}
}

void CharacterAdapter::mouseDown(ui::Widget* widget, const Vector2& position)
{
	if (m_gizmo)
		m_gizmo->mouseDown(widget, this, position);
}

void CharacterAdapter::mouseUp(ui::Widget* widget, const Vector2& position)
{
	if (m_gizmo)
		m_gizmo->mouseUp(widget, this, position);
}

void CharacterAdapter::mouseMove(ui::Widget* widget, const Vector2& position)
{
	if (m_gizmo)
		m_gizmo->mouseMove(widget, this, position);
}

void CharacterAdapter::paint(render::PrimitiveRenderer* primitiveRenderer)
{
	if (m_gizmo)
		m_gizmo->paint(this, primitiveRenderer);
}

void CharacterAdapter::select()
{
	m_selected = true;
}

void CharacterAdapter::deselect()
{
	m_selected = false;
}

bool CharacterAdapter::isSelected() const
{
	return m_selected;
}

bool CharacterAdapter::isChildOfExternal() const
{
	for (CharacterAdapter* i = m_parent; i; i = i->getParent())
	{
		if (is_a< External >(i->getCharacter()))
			return true;
	}
	return false;
}

	}
}
