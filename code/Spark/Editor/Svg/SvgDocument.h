#ifndef traktor_spark_SvgDocument_H
#define traktor_spark_SvgDocument_H

#include "Core/Math/Aabb2.h"
#include "Spark/Editor/Svg/SvgShape.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class SvgDocument : public SvgShape
{
	T_RTTI_CLASS;

public:
	SvgDocument();

	void setViewBox(const Aabb2& viewBox);

	const Aabb2& getViewBox() const;

	void setSize(const Vector2& size);

	const Vector2& getSize() const;

private:
	Aabb2 m_viewBox;
	Vector2 m_size;
};

	}
}

#endif	// traktor_spark_SvgDocument_H
