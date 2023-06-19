/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Spark/BitmapResource.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.BitmapResource", 0, BitmapResource, Bitmap)

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
