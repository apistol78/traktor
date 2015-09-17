#ifndef traktor_spark_Text_H
#define traktor_spark_Text_H

#include "Resource/Id.h"
#include "Spark/Character.h"
#include "Spark/Types.h"

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

class Font;

/*! \brief Text character.
 * \ingroup Spark
 */
class T_DLLCLASS Text : public Character
{
	T_RTTI_CLASS;

public:
	Text();

	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;

	resource::Id< Font > m_font;
	std::wstring m_text;
	float m_height;
	Vector2 m_origin;
	Vector2 m_size;
	Align m_horizontalAlign;
	Align m_verticalAlign;
};

	}
}

#endif	// traktor_spark_Text_H
