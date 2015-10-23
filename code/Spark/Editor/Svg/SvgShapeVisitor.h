#ifndef traktor_spark_SvgShapeVisitor_H
#define traktor_spark_SvgShapeVisitor_H

#include "Core/Object.h"

namespace traktor
{
	namespace spark
	{

class SvgShape;

/*! \brief
 * \ingroup Spark
 */
class SvgShapeVisitor : public Object
{
	T_RTTI_CLASS;

public:
	virtual void enter(SvgShape* shape) = 0;

	virtual void leave(SvgShape* shape) = 0;
};

	}
}

#endif	// traktor_spark_SvgShapeVisitor_H
