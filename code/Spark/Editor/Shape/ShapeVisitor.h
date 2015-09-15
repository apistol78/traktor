#ifndef traktor_spark_ShapeVisitor_H
#define traktor_spark_ShapeVisitor_H

#include "Core/Object.h"

namespace traktor
{
	namespace spark
	{

class Shape;

/*! \brief
 * \ingroup Spark
 */
class ShapeVisitor : public Object
{
	T_RTTI_CLASS;

public:
	virtual void enter(Shape* shape) = 0;

	virtual void leave(Shape* shape) = 0;
};

	}
}

#endif	// traktor_spark_ShapeVisitor_H
