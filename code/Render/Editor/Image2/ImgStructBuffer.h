/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/ImmutableNode.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS ImgStructBuffer : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgStructBuffer();

	bool getPersistent() const;

	virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	bool m_persistent = false;
	uint32_t m_elementCount = 0;
	uint32_t m_elementSize = 0;
};

}
