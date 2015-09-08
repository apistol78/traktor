#ifndef traktor_spark_Character_H
#define traktor_spark_Character_H

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

class CharacterInstance;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Character : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance() const = 0;
};

	}
}

#endif	// traktor_spark_Character_H
