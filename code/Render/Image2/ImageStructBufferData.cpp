/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Image2/ImageStructBuffer.h"
#include "Render/Image2/ImageStructBufferData.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageStructBufferData", 0, ImageStructBufferData, ISerializable)

Ref< const ImageStructBuffer > ImageStructBufferData::createInstance(int32_t instance) const
{
	return new ImageStructBuffer(
		T_FILE_LINE_W,
		getParameterHandle(m_id),
		!m_persistentHandle.empty() ? getParameterHandle(m_persistentHandle + toString(instance)) : 0,
		m_bufferSize
	);
}

void ImageStructBufferData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< std::wstring >(L"persistentHandle", m_persistentHandle);
	s >> Member< uint32_t >(L"bufferSize", m_bufferSize);
}

}
