#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashBitmap.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashBitmap", FlashBitmap, ISerializable)

FlashBitmap::FlashBitmap()
:	m_width(0)
,	m_height(0)
{
}

FlashBitmap::FlashBitmap(uint32_t width, uint32_t height)
:	m_width(width)
,	m_height(height)
{
}

void FlashBitmap::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);
}

	}
}
