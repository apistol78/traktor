/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IThumbnailGenerator.h"

namespace traktor::editor
{

class ThumbnailGenerator : public IThumbnailGenerator
{
	T_RTTI_CLASS;

public:
	explicit ThumbnailGenerator(const Path& thumbsPath);

	virtual Ref< drawing::Image > get(const Path& fileName, int32_t width, int32_t height, Alpha alpha, Gamma gamma) override final;

private:
	Path m_thumbsPath;
};

}
