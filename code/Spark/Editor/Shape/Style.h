#ifndef traktor_spark_Style_H
#define traktor_spark_Style_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace spark
	{

class Gradient;

class Style : public Object
{
	T_RTTI_CLASS;

public:
	typedef unsigned long color_t;

	Style();

	void setFillEnable(bool fillEnable);

	bool getFillEnable() const;
	
	void setFillGradient(Gradient* fillGradient);
	
	Gradient* getFillGradient() const;

	void setFill(color_t fill);

	color_t getFill() const;

	void setStrokeEnable(bool strokeEnable);

	bool getStrokeEnable() const;
	
	void setStrokeGradient(Gradient* strokeGradient);
	
	Gradient* getStrokeGradient() const;

	void setStrokeWidth(float strokeWidth);

	float getStrokeWidth() const;

	void setStroke(color_t stroke);

	color_t getStroke() const;

private:
	bool m_fillEnable;
	Ref< Gradient > m_fillGradient;
	color_t m_fill;
	bool m_strokeEnable;
	Ref< Gradient > m_strokeGradient;
	float m_strokeWidth;
	color_t m_stroke;
};
	
	}
}

#endif	// traktor_spark_Style_H
