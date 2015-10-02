#ifndef traktor_spark_CharacterAdapter_H
#define traktor_spark_CharacterAdapter_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{

class Matrix33;

	namespace spark
	{

class Character;
class CharacterInstance;

class CharacterAdapter : public Object
{
	T_RTTI_CLASS;

public:
	CharacterAdapter();

	const std::wstring& getName() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	Character* getCharacter();

	CharacterInstance* getCharacterInstance();

	CharacterAdapter* getParent();

	const RefArray< CharacterAdapter >& getChildren();

	void select();

	void deselect();

	bool isSelected() const;

	bool isChildOfExternal() const;

private:
	friend class CharacterAdapterBuilder;

	std::wstring m_name;
	Ref< Character > m_character;
	Ref< CharacterInstance > m_characterInstance;
	CharacterAdapter* m_parent;
	RefArray< CharacterAdapter > m_children;
	bool m_selected;
};

	}
}

#endif	// traktor_spark_CharacterAdapter_H
