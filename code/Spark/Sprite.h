#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include <list>
#include <map>
#include "Core/RefArray.h"
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
	const Character* getCharacter(const std::wstring& id) const;

	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const;

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;

	struct T_DLLCLASS Place
	{
		std::wstring name;
		Ref< Character > character;
		Matrix33 transform;

		Place();

		void serialize(ISerializer& s);
	};

	RefArray< IComponent > m_components;
	resource::Id< Shape > m_shape;
	std::map< std::wstring, Ref< Character > > m_characters;
	std::list< Place > m_place;
};

	}
}

#endif	// traktor_spark_Sprite_H
