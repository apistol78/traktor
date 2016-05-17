#ifndef traktor_editor_TextEditorDialog_H
#define traktor_editor_TextEditorDialog_H

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

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;

private:
	Ref< ui::RichEdit > m_edit;
};

	}
}

#endif	// traktor_editor_TextEditorDialog_H
