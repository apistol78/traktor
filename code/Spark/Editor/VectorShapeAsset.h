#ifndef traktor_spark_VectorShapeAsset_H
#define traktor_spark_VectorShapeAsset_H

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
class T_DLLCLASS VectorShapeAsset : public ShapeAsset
{
	T_RTTI_CLASS;

public:
	enum PivotType
	{
		PtViewTopLeft,
		PtViewCenter,
		PtShapeCenter
	};

	VectorShapeAsset();

	virtual void serialize(ISerializer& s);

private:
	friend class VectorShapePipeline;

	PivotType m_pivot;
};

	}
}

#endif	// traktor_spark_VectorShapeAsset_H
