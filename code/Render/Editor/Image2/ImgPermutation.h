/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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

class T_DLLCLASS ImgPermutation : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgPermutation();

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

	std::wstring m_name;
};

}
