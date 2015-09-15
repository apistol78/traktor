#include "Spark/Tween.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Tween", Tween, Object)

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Tween.IListener", Tween::IListener, Object)

Tween::Tween(float from, float to, float duration, IListener* listener)
{
}

	}
}
