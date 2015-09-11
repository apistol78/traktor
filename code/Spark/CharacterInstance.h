#ifndef traktor_spark_CharacterInstance_H
#define traktor_spark_CharacterInstance_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace spark
	{

class IComponentInstance;

/*! \brief
 * \ingroup Spark
 */
class CharacterInstance : public Object
{
	T_RTTI_CLASS;

public:
	CharacterInstance();

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	void addComponent(IComponentInstance* component);

	const RefArray< IComponentInstance >& getComponents() const;

	virtual void update();

	virtual void render(render::RenderContext* renderContext) const = 0;

private:
	Matrix33 m_transform;
	RefArray< IComponentInstance > m_components;
};

	}
}

#endif	// traktor_spark_CharacterInstance_H
