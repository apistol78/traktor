#ifndef traktor_spark_Tween_H
#define traktor_spark_Tween_H

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

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Tween : public Object
{
	T_RTTI_CLASS;

public:
	class IListener : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual void notify(float value) = 0;
	};

	Tween(float from, float to, float duration, IListener* listener);
};

	}
}

#endif	// traktor_spark_Tween_H
