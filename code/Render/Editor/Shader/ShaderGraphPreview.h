/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Drawing/Image.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Database;

}

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IEditor;

}

namespace traktor::render
{

class ShaderGraph;

class T_DLLCLASS ShaderGraphPreview : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphPreview(const editor::IEditor* editor);

	Ref< drawing::Image > generate(const ShaderGraph* shaderGraph, int32_t width, int32_t height) const;

private:
	std::wstring m_assetPath;
	Ref< db::Database > m_database;
};

}
