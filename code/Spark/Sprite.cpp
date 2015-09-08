#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Sprite", Sprite, Character)

Ref< CharacterInstance > Sprite::createInstance() const
{
	return new SpriteInstance(this);
}

	}
}
