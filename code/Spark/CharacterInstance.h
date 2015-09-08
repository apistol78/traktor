#ifndef traktor_spark_CharacterInstance_H
#define traktor_spark_CharacterInstance_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class CharacterInstance : public Object
{
	T_RTTI_CLASS;

public:
	virtual void render(render::RenderContext* renderContext) const = 0;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

private:
	Matrix33 m_transform;
};

	}
}

#endif	// traktor_spark_CharacterInstance_H
