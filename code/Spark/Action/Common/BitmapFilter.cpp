#include "Spark/Action/Common/BitmapFilter.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.BitmapFilter", BitmapFilter, ActionObjectRelay)

BitmapFilter::BitmapFilter(const char* const prototype)
:	ActionObjectRelay(prototype)
{
}

	}
}
