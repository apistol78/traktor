/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/Bitmap.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Bitmap", Bitmap, ISerializable)

Bitmap::Bitmap()
:	m_x(0)
,	m_y(0)
,	m_width(0)
,	m_height(0)
{
}

Bitmap::Bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
:	m_x(x)
,	m_y(y)
,	m_width(width)
,	m_height(height)
{
}

void Bitmap::setCacheObject(IRefCount* cacheObject) const
{
	m_cacheObject = cacheObject;
}

void Bitmap::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"x", m_x);
	s >> Member< uint32_t >(L"y", m_y);
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);
}

	}
}
