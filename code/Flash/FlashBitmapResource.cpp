#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashBitmapResource.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBitmapResource", 0, FlashBitmapResource, FlashBitmap)

FlashBitmapResource::FlashBitmapResource()
{
}

FlashBitmapResource::FlashBitmapResource(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const Guid& resourceId)
:	FlashBitmap(x, y, width, height)
,	m_resourceId(resourceId)
{
}

void FlashBitmapResource::serialize(ISerializer& s)
{
	FlashBitmap::serialize(s);
	s >> Member< Guid >(L"resourceId", m_resourceId);
}

	}
}
