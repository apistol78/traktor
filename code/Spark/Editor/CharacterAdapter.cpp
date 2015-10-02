#include "Spark/Character.h"
#include "Spark/CharacterInstance.h"
#include "Spark/External.h"
#include "Spark/Editor/CharacterAdapter.h"

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
