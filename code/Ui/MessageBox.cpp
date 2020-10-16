#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/MessageBox.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MessageBox", MessageBox, ConfigDialog)

bool MessageBox::create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	int dialogStyle = WsCenterParent | WsSystemBox | WsCloseBox | WsCaption;
	if (style & MbYesNo)
		dialogStyle |= ConfigDialog::WsYesNoButtons;

	if (!ConfigDialog::create(
		parent,
		caption,
		dpi96(200),
		dpi96(100),
		dialogStyle,
		new TableLayout(L"*", L"*", dpi96(4), dpi96(4))
	))
		return false;

	Ref< Static > staticMessage = new Static();
	staticMessage->create(this, message);

	fit(Container::FaBoth);
	return true;
}

int MessageBox::show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	MessageBox mb;

	if (!mb.create(parent, message, caption, style))
		return DrCancel;

	int result = mb.showModal();

	mb.destroy();

	return result;
}

int MessageBox::show(const std::wstring& message, const std::wstring& caption, int style)
{
	return show(nullptr, message, caption, style);
}

	}
}
