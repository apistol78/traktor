#ifndef traktor_spark_IComponentInstance_H
#define traktor_spark_IComponentInstance_H

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

/*! \brief Character component instance.
 * \ingroup Spark
 */
class T_DLLCLASS IComponentInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update() = 0;
};

	}
}

#endif	// traktor_spark_IComponentInstance_H
