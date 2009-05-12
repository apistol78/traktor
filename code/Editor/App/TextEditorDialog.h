#ifndef traktor_editor_TextEditorDialog_H
#define traktor_editor_TextEditorDialog_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(TextEditorDialog)

public:
	bool create(ui::Widget* parent, const std::wstring& initialText);

	std::wstring getText() const;

private:
	Ref< ui::RichEdit > m_edit;
};

	}
}

#endif	// traktor_editor_TextEditorDialog_H
