#include "Render/TextureResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.TextureResource", TextureResource, Serializable)

bool TextureResource::serialize(Serializer& s)
{
	return true;
}

	}
}
