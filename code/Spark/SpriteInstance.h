#ifndef traktor_spark_SpriteInstance_H
#define traktor_spark_SpriteInstance_H

#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "Spark/CharacterInstance.h"

namespace traktor
{
	namespace spark
	{

class Shape;
class Sprite;

/*! \brief
 * \ingroup Spark
 */
class SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	SpriteInstance();

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	friend class Sprite;

	Ref< const Sprite > m_sprite;
	const CharacterInstance* m_parent;
	resource::Proxy< Shape > m_shape;
	RefArray< CharacterInstance > m_children;
};

	}
}

#endif	// traktor_spark_SpriteInstance_H
