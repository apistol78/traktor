#ifndef traktor_spark_ICharacterBuilder_H
#define traktor_spark_ICharacterBuilder_H

#include "Core/Object.h"

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

class Character;
class CharacterData;
class Context;

/*! \brief Character builder interface.
 * \ingroup Spark
 */
class T_DLLCLASS ICharacterBuilder : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Character > create(const Context* context, const CharacterData* characterData, const Character* parent, const std::wstring& name) const = 0;
};

	}
}

#endif	// traktor_spark_ICharacterBuilder_H
