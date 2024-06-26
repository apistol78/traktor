/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslImage.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslImage", GlslImage, GlslResource)

GlslImage::GlslImage(const std::wstring& name, Set set, uint8_t stages, GlslType uniformType, bool indexed)
:	GlslResource(name, set, stages)
,	m_uniformType(uniformType)
,	m_indexed(indexed)
{
}

int32_t GlslImage::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
