/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IBrowsePreview.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Texture asset preview generator.
 * \ingroup Render
 */
class T_DLLCLASS TextureBrowsePreview : public editor::IBrowsePreview
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const override final;

	virtual Ref< ui::Bitmap > generate(editor::IEditor* editor, db::Instance* instance) const override final;
};

}
