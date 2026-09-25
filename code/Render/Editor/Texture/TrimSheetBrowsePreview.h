/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Editor/IBrowsePreview.h"

#include <string>

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

namespace traktor::render
{

/*! Browse preview of trim sheet setups and textures.
 * \ingroup Render
 */
class T_DLLCLASS TrimSheetBrowsePreview : public editor::IBrowsePreview
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const override final;

	virtual Ref< ui::IBitmap > generate(editor::IEditor* editor, db::Instance* instance, int32_t size) const override final;

	/*! Generate opaque preview image of a trim sheet texture, or albedo of a trim sheet setup.
	 *
	 * \return Preview image, null if instance isn't a trim sheet or it cannot be composed.
	 */
	static Ref< drawing::Image > generateImage(const std::wstring& assetPath, db::Database* database, const Guid& instanceGuid, int32_t width, int32_t height);
};

}
