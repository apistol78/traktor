#ifndef traktor_drawing_Color_H
#define traktor_drawing_Color_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief High range color.
 * \ingroup Drawing
 */
class T_DLLCLASS Color : public Object
{
	T_RTTI_CLASS;

public:
	Color();

	Color(const Color& src);

	Color(float red, float green, float blue, float alpha = 0.0f);

	float get(int channel) const;

	float getRed() const;

	float getGreen() const;

	float getBlue() const;

	float getAlpha() const;

	void set(int channel, float value);
	
	void set(float red, float green, float blue, float alpha = 0.0f);

	void setRed(float red);

	void setGreen(float green);

	void setBlue(float blue);

	void setAlpha(float alpha);

	Color& operator = (const Color& src);

	Color operator + (const Color& r) const;

	Color operator - (const Color& r) const;

	Color operator * (const Color& r) const;

	Color operator * (float r) const;

	Color operator / (float r) const;

	Color& operator += (const Color& r);

	Color& operator -= (const Color& r);

	Color& operator *= (float r);

	Color& operator /= (float r);

	bool operator == (const Color& r) const;

	bool operator != (const Color& r) const;

private:
	union
	{
		struct
		{
			float m_red;
			float m_green;
			float m_blue;
			float m_alpha;
		};
		float m_channel[4];
	};
};

	}
}

#endif	// traktor_drawing_Color_H
