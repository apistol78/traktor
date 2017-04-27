/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SvgShape_H
#define traktor_spark_SvgShape_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace spark
	{

class SvgShapeVisitor;
class SvgStyle;

/*! \brief
 * \ingroup Spark
 */
class SvgShape : public Object
{
	T_RTTI_CLASS;

public:
	SvgShape();

	void setStyle(const SvgStyle* style);

	const SvgStyle* getStyle() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	void addChild(SvgShape* shape);

	virtual void visit(SvgShapeVisitor* shapeVisitor);

private:
	Ref< const SvgStyle > m_style;
	Matrix33 m_transform;
	RefArray< SvgShape > m_children;
};

	}
}

#endif	// traktor_spark_Shape_H
