/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SvgStyle_H
#define traktor_spark_SvgStyle_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4ub.h"

namespace traktor
{
	namespace spark
	{

class SvgGradient;

/*! \brief
 * \ingroup Spark
 */
class SvgStyle : public Object
{
	T_RTTI_CLASS;

public:
	SvgStyle();

	void setFillEnable(bool fillEnable);

	bool getFillEnable() const;
	
	void setFillGradient(const SvgGradient* fillGradient);
	
	const SvgGradient* getFillGradient() const;

	void setFill(const Color4ub& fill);

	const Color4ub& getFill() const;

	void setStrokeEnable(bool strokeEnable);

	bool getStrokeEnable() const;
	
	void setStrokeGradient(const SvgGradient* strokeGradient);
	
	const SvgGradient* getStrokeGradient() const;

	void setStrokeWidth(float strokeWidth);

	float getStrokeWidth() const;

	void setStroke(const Color4ub& stroke);

	const Color4ub& getStroke() const;

	void setOpacity(float opacity);

	float getOpacity() const;

	bool operator == (const SvgStyle& other) const;

private:
	bool m_fillEnable;
	Ref< const SvgGradient > m_fillGradient;
	Color4ub m_fill;
	bool m_strokeEnable;
	Ref< const SvgGradient > m_strokeGradient;
	float m_strokeWidth;
	Color4ub m_stroke;
	float m_opacity;
};
	
	}
}

#endif	// traktor_spark_SvgStyle_H
