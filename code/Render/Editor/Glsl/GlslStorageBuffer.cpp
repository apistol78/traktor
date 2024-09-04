/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslStorageBuffer", GlslStorageBuffer, GlslResource)

GlslStorageBuffer::GlslStorageBuffer(const std::wstring& name, Set set, uint8_t stages)
:	GlslResource(name, set, stages)
{
}

bool GlslStorageBuffer::add(const std::wstring& elementName, DataType elementType, int32_t elementLength)
{
	m_elements.push_back({ elementName, elementType, elementLength });
	return true;
}

int32_t GlslStorageBuffer::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
