#include "Editor/App/TextEditorDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/RichEdit.h"
#include "I18N/Text.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.TextEditorDialog", TextEditorDialog, ui::ConfigDialog)

bool TextEditorDialog::create(ui::Widget* parent, const std::wstring& initialText)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"TEXT_EDIT"), 500, 400, ui::ConfigDialog::WsDefaultResizable, gc_new< ui::FloodLayout >()))
		return false;

	m_edit = gc_new< ui::RichEdit >();
	if (!m_edit->create(this, initialText, ui::WsClientBorder))
		return false;

	m_edit->setFont(ui::Font(L"Courier New", 14));

	update();

	return true;
}

std::wstring TextEditorDialog::getText() const
{
	return m_edit->getText();
}

	}
}
