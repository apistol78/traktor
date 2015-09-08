#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

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

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance() const;
};

	}
}

#endif	// traktor_spark_Sprite_H
