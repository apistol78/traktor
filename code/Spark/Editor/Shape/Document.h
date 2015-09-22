#ifndef traktor_spark_Document_H
#define traktor_spark_Document_H

#include "Core/Math/Aabb2.h"
#include "Spark/Editor/Shape/Shape.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class Document : public Shape
{
	T_RTTI_CLASS;

public:
	Document();

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

#endif	// traktor_spark_Document_H
