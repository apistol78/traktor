#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashBitmap.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashBitmap", FlashBitmap, ISerializable)

FlashBitmap::FlashBitmap()
:	m_x(0)
,	m_y(0)
,	m_width(0)
,	m_height(0)
{
}

FlashBitmap::FlashBitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
:	m_x(x)
,	m_y(y)
,	m_width(width)
,	m_height(height)
{
}

void FlashBitmap::setCacheObject(IRefCount* cacheObject) const
{
	m_cacheObject = cacheObject;
}

void FlashBitmap::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"x", m_x);
	s >> Member< uint32_t >(L"y", m_y);
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);
}

	}
}
