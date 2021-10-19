#pragma once

#include "Svg/Shape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace drawing
    {

class Image;

    }

	namespace svg
	{

/*! SVG image shape.
 * \ingroup SVG
 */
class T_DLLCLASS ImageShape : public Shape
{
	T_RTTI_CLASS;

public:
	explicit ImageShape(const Vector2& position, const Vector2& size, const drawing::Image* image);

	const Vector2& getPosition() const;

    const Vector2& getSize() const;

    const drawing::Image* getImage() const;

private:
	Vector2 m_position;
    Vector2 m_size;
    Ref< const drawing::Image > m_image;
};

	}
}
