#pragma once

#include "Spark/CharacterInstance.h"

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

class Shape;

/*! \brief Flash shape instance.
 * \ingroup Spark
 */
class T_DLLCLASS ShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	ShapeInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape);

	const Shape* getShape() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const Shape > m_shape;
};

	}
}

