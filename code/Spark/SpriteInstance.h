#ifndef traktor_spark_SpriteInstance_H
#define traktor_spark_SpriteInstance_H

#include "Spark/CharacterInstance.h"

namespace traktor
{
	namespace spark
	{

class Sprite;

/*! \brief
 * \ingroup Spark
 */
class SpriteInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	SpriteInstance(const Sprite* Sprite);

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	Ref< const Sprite > m_Sprite;
};

	}
}

#endif	// traktor_spark_SpriteInstance_H
