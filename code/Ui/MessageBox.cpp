#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/MessageBox.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IMessageBox.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MessageBox", MessageBox, EventSubject)

MessageBox::MessageBox()
:	m_messageBox(0)
{
}

MessageBox::~MessageBox()
{
	T_ASSERT_M (!m_messageBox, L"MessageBox not destroyed");
}

bool MessageBox::create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	m_messageBox = Application::getInstance()->getWidgetFactory()->createMessageBox(this);
	if (!m_messageBox)
	{
		log::error << L"Failed to create native widget peer (MessageBox)" << Endl;
		return false;
	}

	if (!m_messageBox->create(parent ? parent->getIWidget() : 0, message, caption, style))
		return false;

	return true;
}

void MessageBox::destroy()
{
	if (m_messageBox)
	{
		m_messageBox->destroy();
		m_messageBox = 0;
	}
}

int MessageBox::showModal()
{
	T_ASSERT (m_messageBox);
	return m_messageBox->showModal();
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
	return show(0, message, caption, style);
}

	}
}
