#pragma once

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

class Movie;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Optimizer : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Movie > merge(const Movie* movie) const;

	void triangulate(Movie* movie, bool discardPaths) const;
};

	}
}

