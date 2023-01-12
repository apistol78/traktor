/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class AutoPropertyList;
class ListBox;
class ToolBarButtonClickEvent;

}

namespace traktor::render
{

class TextureAsset;

class TextureBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit TextureBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	bool showModal(RefArray< TextureAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::ListBox > m_textureList;
	Ref< ui::AutoPropertyList > m_texturePropertyList;

	void addTexture();

	void removeTexture();

	void eventTextureListToolClick(ui::ToolBarButtonClickEvent* event);

	void eventTextureListSelect(ui::SelectionChangeEvent* event);
};

}
