#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class RichEdit;

	}

	namespace editor
	{

class TextEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, const std::wstring& initialText);

	virtual std::wstring getText() const override final;

private:
	Ref< ui::RichEdit > m_edit;
};

	}
}

