#include "Spray/PointSetResource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSetResource", 0, PointSetResource, ISerializable)

bool PointSetResource::serialize(ISerializer& s)
{
	return true;
}
	
	}
}
