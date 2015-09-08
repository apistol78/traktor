#ifndef traktor_spark_Style_H
#define traktor_spark_Style_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4ub.h"

namespace traktor
{
	namespace spark
	{

class Gradient;

class Style : public Object
{
	T_RTTI_CLASS;

public:
	Style();

	void setFillEnable(bool fillEnable);

	bool getFillEnable() const;
	
	void setFillGradient(Gradient* fillGradient);
	
	Gradient* getFillGradient() const;

	void setFill(const Color4ub& fill);

	const Color4ub& getFill() const;

	void setStrokeEnable(bool strokeEnable);

	bool getStrokeEnable() const;
	
	void setStrokeGradient(Gradient* strokeGradient);
	
	Gradient* getStrokeGradient() const;

	void setStrokeWidth(float strokeWidth);

	float getStrokeWidth() const;

	void setStroke(const Color4ub& stroke);

	const Color4ub& getStroke() const;

private:
	bool m_fillEnable;
	Ref< Gradient > m_fillGradient;
	Color4ub m_fill;
	bool m_strokeEnable;
	Ref< Gradient > m_strokeGradient;
	float m_strokeWidth;
	Color4ub m_stroke;
};
	
	}
}

#endif	// traktor_spark_Style_H
