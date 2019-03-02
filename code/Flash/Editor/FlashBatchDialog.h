#pragma once

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

class AutoPropertyList;
class ListBox;
class ToolBarButtonClickEvent;

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

	virtual void destroy() override final;

	bool showModal(RefArray< FlashMovieAsset >& outAssets);

private:
	editor::IEditor* m_editor;
	Ref< ui::ListBox > m_movieList;
	Ref< ui::AutoPropertyList > m_moviePropertyList;

	void addTexture();

	void removeTexture();

	void eventTextureListToolClick(ui::ToolBarButtonClickEvent* event);

	void eventTextureListSelect(ui::SelectionChangeEvent* event);
};

	}
}

