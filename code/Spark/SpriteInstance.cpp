#include "Spark/SpriteInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SpriteInstance", SpriteInstance, CharacterInstance)

SpriteInstance::SpriteInstance(const Sprite* Sprite)
:	m_Sprite(Sprite)
{
}

void SpriteInstance::render(render::RenderContext* renderContext) const
{
}

	}
}
