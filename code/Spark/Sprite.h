#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Id.h"
#include "Spark/Character.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class IComponent;
class Shape;

/*! \brief Sprite character.
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	struct NamedCharacter
	{
		std::wstring name;
		Ref< Character > character;
	};

	const Character* getCharacter(const std::wstring& id) const;

	void place(const std::wstring& name, Character* character);

	const AlignedVector< NamedCharacter >& getDictionary() { return m_dictionary; }

	const AlignedVector< NamedCharacter >& getFrame() const { return m_frame; }

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;
	friend class SpriteFactory;

	RefArray< IComponent > m_components;
	resource::Id< Shape > m_shape;
	AlignedVector< NamedCharacter > m_dictionary;
	AlignedVector< NamedCharacter > m_frame;
};

	}
}

#endif	// traktor_spark_Sprite_H
