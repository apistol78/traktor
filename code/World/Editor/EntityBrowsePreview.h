/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Editor/IBrowsePreview.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Entity preview generator.
 * \ingroup Mesh
 */
class T_DLLCLASS EntityBrowsePreview : public editor::IBrowsePreview
{
	T_RTTI_CLASS;

public:
	EntityBrowsePreview();

	virtual TypeInfoSet getPreviewTypes() const override final;

	virtual Ref< ui::IBitmap > generate(editor::IEditor* editor, db::Instance* instance) const override final;

private:
	RefArray< const IBrowsePreview > m_browsePreview;
};

}
