#ifndef traktor_spark_ShapeAsset_H
#define traktor_spark_ShapeAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
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
class T_DLLCLASS ShapeAsset : public editor::Asset
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_spark_ShapeAsset_H
