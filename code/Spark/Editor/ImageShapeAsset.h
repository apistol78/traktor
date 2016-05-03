#ifndef traktor_spark_ImageShapeAsset_H
#define traktor_spark_ImageShapeAsset_H

#include "Spark/Editor/ShapeAsset.h"

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
class T_DLLCLASS ImageShapeAsset : public ShapeAsset
{
	T_RTTI_CLASS;

public:
	enum PivotType
	{
		PtTopLeft,
		PtCenter
	};

	ImageShapeAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ImageShapePipeline;

	PivotType m_pivot;
};

	}
}

#endif	// traktor_spark_ImageShapeAsset_H
