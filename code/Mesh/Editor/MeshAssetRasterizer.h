/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace editor
	{

class IEditor;

	}

	namespace mesh
	{

class MeshAsset;

class MeshAssetRasterizer : public Object
{
	T_RTTI_CLASS;

public:
	bool generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const;
};

	}
}
