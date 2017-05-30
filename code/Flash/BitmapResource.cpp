/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/BitmapResource.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.BitmapResource", 0, BitmapResource, Bitmap)

BitmapResource::BitmapResource()
:	m_atlasWidth(0)
,	m_atlasHeight(0)
{
}

BitmapResource::BitmapResource(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t atlasWidth, uint32_t atlasHeight, const Guid& resourceId)
:	Bitmap(x, y, width, height)
,	m_atlasWidth(atlasWidth)
,	m_atlasHeight(atlasHeight)
,	m_resourceId(resourceId)
{
}

void BitmapResource::serialize(ISerializer& s)
{
	Bitmap::serialize(s);
	s >> Member< uint32_t >(L"atlasWidth", m_atlasWidth);
	s >> Member< uint32_t >(L"atlasHeight", m_atlasHeight);
	s >> Member< Guid >(L"resourceId", m_resourceId);
}

	}
}
