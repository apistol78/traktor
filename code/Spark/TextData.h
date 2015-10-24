#ifndef traktor_spark_TextData_H
#define traktor_spark_TextData_H

#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
#include "Spark/CharacterData.h"
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

/*! \brief TextData character.
 * \ingroup Spark
 */
class T_DLLCLASS TextData : public CharacterData
{
	T_RTTI_CLASS;

public:
	TextData();

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;
	friend class TextFactory;

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

#endif	// traktor_spark_TextData_H
