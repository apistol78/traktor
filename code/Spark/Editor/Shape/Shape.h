#ifndef traktor_spark_Shape_H
#define traktor_spark_Shape_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace spark
	{

class ShapeVisitor;
class Style;

/*! \brief
 * \ingroup Spark
 */
class Shape : public Object
{
	T_RTTI_CLASS;

public:
	Shape();

	void setStyle(Style* style);

	Style* getStyle() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	void addChild(Shape* shape);

	virtual void visit(ShapeVisitor* shapeVisitor);

private:
	Ref< Style > m_style;
	Matrix33 m_transform;
	RefArray< Shape > m_children;
};

	}
}

#endif	// traktor_spark_Shape_H
