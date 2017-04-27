/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashBatchDialog_H
#define traktor_flash_FlashBatchDialog_H

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

	namespace flash
	{

class FlashMovieAsset;

class FlashBatchDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	FlashBatchDialog(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool showModal(RefArray< FlashMovieAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::ListBox > m_movieList;
	Ref< ui::custom::AutoPropertyList > m_moviePropertyList;

	void addTexture();

	void removeTexture();

	void eventTextureListToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTextureListSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_flash_FlashBatchDialog_H
