/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslStorageBuffer.h"

#include "Core/Misc/Murmur3.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslStorageBuffer", GlslStorageBuffer, GlslResource)

GlslStorageBuffer::GlslStorageBuffer(const std::wstring& name, const std::wstring& structTypeName, Set set, uint8_t stages, bool indexed)
	: GlslResource(name, set, stages)
	, m_structTypeName(structTypeName)
	, m_indexed(indexed)
{
}

int32_t GlslStorageBuffer::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.feed(m_structTypeName);
	cs.end();
	return (int32_t)cs.get();
}

}
