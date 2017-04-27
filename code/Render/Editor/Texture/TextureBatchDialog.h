/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TextureBatchDialog_H
#define traktor_render_TextureBatchDialog_H

#include "Core/RefArray.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
		namespace custom
		{

class AutoPropertyList;
class ListBox;
class ToolBarButtonClickEvent;

		}
	}

	namespace render
	{

class TextureAsset;

class TextureBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	TextureBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool showModal(RefArray< TextureAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::ListBox > m_textureList;
	Ref< ui::custom::AutoPropertyList > m_texturePropertyList;

	void addTexture();

	void removeTexture();

	void eventTextureListToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTextureListSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_render_TextureBatchDialog_H
