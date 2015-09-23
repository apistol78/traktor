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

public:
	enum PivotType
	{
		PtViewTopLeft,
		PtViewCenter,
		PtShapeCenter
	};

	ShapeAsset();

	virtual void serialize(ISerializer& s);

private:
	friend class ShapePipeline;

	float m_cubicApproximationError;
	PivotType m_pivot;
};

	}
}

#endif	// traktor_spark_ShapeAsset_H
