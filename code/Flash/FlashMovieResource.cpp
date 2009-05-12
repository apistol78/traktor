#include "Flash/FlashMovieResource.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.flash.FlashMovieResource", FlashMovieResource, Serializable)

bool FlashMovieResource::serialize(Serializer& s)
{
	return true;
}

	}
}
