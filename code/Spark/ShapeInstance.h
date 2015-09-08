#ifndef traktor_spark_ShapeInstance_H
#define traktor_spark_ShapeInstance_H

#include "Spark/CharacterInstance.h"

namespace traktor
{
	namespace spark
	{

class Shape;

/*! \brief
 * \ingroup Spark
 */
class ShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	ShapeInstance(const Shape* shape);

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	Ref< const Shape > m_shape;
};

	}
}

#endif	// traktor_spark_ShapeInstance_H
