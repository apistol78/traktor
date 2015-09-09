#include "Spark/Shape.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance()
:	m_parent(0)
{
}

void SpriteInstance::render(render::RenderContext* renderContext) const
{
	if (m_shape)
		m_shape->render(renderContext, getTransform());

	for (RefArray< CharacterInstance >::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->render(renderContext);
}

	}
}
