/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::ui
{

class Bitmap;

}

namespace traktor::editor
{

class IEditor;

/*! Browse instance preview generator interface.
 * \ingroup Editor
 */
class T_DLLCLASS IBrowsePreview : public Object
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getPreviewTypes() const = 0;

	virtual Ref< ui::Bitmap > generate(IEditor* editor, db::Instance* instance) const = 0;
};

}
