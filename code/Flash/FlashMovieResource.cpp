#include "Flash/FlashMovieResource.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashMovieResource", 0, FlashMovieResource, ISerializable)

bool FlashMovieResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
