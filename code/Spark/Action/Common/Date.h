#pragma once

#include "Spark/Action/ActionObjectRelay.h"

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

/*! ActionScript date.
 * \ingroup Spark
 */
class T_DLLCLASS Date : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Date();
};

	}
}

