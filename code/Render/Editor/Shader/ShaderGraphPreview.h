/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Drawing/Image.h"

#include <map>
#include <string>
#include <tuple>

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

	explicit ShaderGraphPreview(const std::wstring& assetPath, db::Database* database);

	//! Generate preview; previews are kept for the lifetime of this object and returned as copies.
	Ref< const drawing::Image > generate(const ShaderGraph* shaderGraph, int32_t width, int32_t height) const;

private:
	struct Preview
	{
		Semaphore lock;
		Ref< const drawing::Image > image;
		bool generated = false;
	};

	std::wstring m_assetPath;
	Ref< db::Database > m_database;
	mutable Semaphore m_lock;
	mutable std::map< std::tuple< uint32_t, int32_t, int32_t >, Preview > m_previews;

	Ref< const drawing::Image > generateUncached(const ShaderGraph* shaderGraph, int32_t width, int32_t height) const;
};

}
