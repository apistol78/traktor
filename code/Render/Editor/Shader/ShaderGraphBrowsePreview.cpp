/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphBrowsePreview.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Ui/Bitmap.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphBrowsePreview", 0, ShaderGraphBrowsePreview, editor::IBrowsePreview)

TypeInfoSet ShaderGraphBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< ShaderGraph >();
}

Ref< ui::IBitmap > ShaderGraphBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
	if (!shaderGraph)
		return nullptr;

	const int32_t w = 64;
	const int32_t h = 64;
	const int32_t m = 4;
	const float sw = (float)100 / 100.0f;
	const float ns = (float)200 / 100.0f;

	Ref< drawing::Image > shaderGraphThumb = ShaderGraphPreview(editor).generate(shaderGraph, w, h);
	if (shaderGraphThumb)
		return new ui::Bitmap(shaderGraphThumb);
	else
		return nullptr;
}

}
