/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Image2/ImageStructBuffer.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageStructBuffer", ImageStructBuffer, Object)

ImageStructBuffer::ImageStructBuffer(const std::wstring& name, img_handle_t id, handle_t persistentHandle, uint32_t bufferSize)
:	m_name(name)
,	m_id(id)
,	m_persistentHandle(persistentHandle)
,	m_bufferSize(bufferSize)
{
}

const std::wstring& ImageStructBuffer::getName() const
{
	return m_name;
}

img_handle_t ImageStructBuffer::getId() const
{
	return m_id;
}

handle_t ImageStructBuffer::getPersistentHandle() const
{
	return m_persistentHandle;
}

}
