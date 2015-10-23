#ifndef traktor_spark_VectorShape_H
#define traktor_spark_VectorShape_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
#include "Spark/Path.h"

namespace traktor
{
	namespace spark
	{

/*! \brief Intermediate shape
 * \ingroup Spark
 *
 * Intermediate representation of a shape used by
 * the VectorShapePipeline to produce the output.
 */
class VectorShape : public Object
{
	T_RTTI_CLASS;

public:
	struct FillStyle
	{
		Color4f color;
	};

	struct LineStyle
	{
		Color4f color;
		float width;
	};

	struct PathWithStyle
	{
		Path path;
		int32_t fillStyle;
		int32_t lineStyle;
	};

	void setBounds(const Aabb2& bounds);

	const Aabb2& getBounds() const;

	int32_t addFillStyle(const Color4f& color);

	int32_t addLineStyle(const Color4f& color, float width);

	void fill(const Path& path, int32_t fillStyle);

	void stroke(const Path& path, int32_t lineStyle);

	const FillStyle& getFillStyle(int32_t index) const { return m_fillStyles[index]; }

	const LineStyle& getLineStyle(int32_t index) const { return m_lineStyles[index]; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< PathWithStyle >& getPaths() const { return m_paths; }

private:
	Aabb2 m_bounds;
	AlignedVector< FillStyle > m_fillStyles;
	AlignedVector< LineStyle > m_lineStyles;
	AlignedVector< PathWithStyle > m_paths;
};

	}
}

#endif	// traktor_spark_VectorShape_H
