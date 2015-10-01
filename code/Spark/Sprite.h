#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
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
	struct T_DLLCLASS Place
	{
		std::wstring name;
		Ref< Character > character;
		Matrix33 transform;

		Place();

		void serialize(ISerializer& s);
	};

	const Character* getCharacter(const std::wstring& id) const;

	void place(const std::wstring& name, Character* character, const Matrix33& transform);

	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer, bool createComponents) const;

	virtual void serialize(ISerializer& s);

	const SmallMap< std::wstring, Ref< Character > >& getCharacters() { return m_characters; }

	const AlignedVector< Place >& getPlacements() const { return m_place; }

private:
	friend class CharacterPipeline;

	RefArray< IComponent > m_components;
	resource::Id< Shape > m_shape;
	SmallMap< std::wstring, Ref< Character > > m_characters;
	AlignedVector< Place > m_place;
};

	}
}

#endif	// traktor_spark_Sprite_H
