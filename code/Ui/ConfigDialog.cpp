#include "Ui/ConfigDialog.h"
#include "Ui/Button.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ConfigDialog", ConfigDialog, Dialog)

bool ConfigDialog::create(Widget* parent, const std::wstring& text, int width, int height, int style, Layout* layout)
{
	if (!Dialog::create(parent, text, width, height, style, layout))
		return false;
	
	m_ok = new Button();
	m_ok->create(this, L"Ok", Button::WsDefaultButton);
	m_ok->addClickEventHandler(createMethodHandler(this, &ConfigDialog::eventClick));
	m_ok->unlink();
	
	m_cancel = new Button();
	m_cancel->create(this, L"Cancel");
	m_cancel->addClickEventHandler(createMethodHandler(this, &ConfigDialog::eventClick));
	m_cancel->unlink();

	if (style & WsApplyButton)
	{
		m_apply = new Button();
		m_apply->create(this, L"Apply");
		m_apply->addClickEventHandler(createMethodHandler(this, &ConfigDialog::eventClick));
		m_apply->unlink();
	}

	update();
	
	return true;
}

void ConfigDialog::destroy()
{
	if (m_apply)
	{
		m_apply->destroy();
		m_apply = 0;
	}

	if (m_cancel)
	{
		m_cancel->destroy();
		m_cancel = 0;
	}

	if (m_ok)
	{
		m_ok->destroy();
		m_ok = 0;
	}

	Dialog::destroy();
}

void ConfigDialog::update(const Rect* rc, bool immediate)
{
	Rect rcInner = getInnerRect();
	
	Size ok = m_ok->getPreferedSize();
	Size cancel = m_cancel->getPreferedSize();

	if (m_apply)
	{
		Size apply = m_apply->getPreferedSize();

		m_ok->setRect(Rect(Point(rcInner.right - ok.cx - cancel.cx - apply.cx - 3 * 4, rcInner.bottom + 4 + 4), ok));
		m_cancel->setRect(Rect(Point(rcInner.right - cancel.cx - apply.cx - 2 * 4, rcInner.bottom + 4 + 4), cancel));
		m_apply->setRect(Rect(Point(rcInner.right - apply.cx - 1 * 4, rcInner.bottom + 4 + 4), apply));
	}
	else
	{
		m_ok->setRect(Rect(Point(rcInner.right - ok.cx - cancel.cx - 2 * 4, rcInner.bottom + 4 + 4), ok));
		m_cancel->setRect(Rect(Point(rcInner.right - cancel.cx - 1 * 4, rcInner.bottom + 4 + 4), cancel));
	}

	Dialog::update(rc, immediate);
}

Rect ConfigDialog::getInnerRect() const
{
	Rect rc = Dialog::getInnerRect();
	if (m_ok)
		rc.bottom -= m_ok->getPreferedSize().cy + 2 * 4 + 4;
	return rc;
}

void ConfigDialog::addClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiClick, eventHandler);
}

void ConfigDialog::eventClick(Event* event)
{
	if (hasEventHandler(EiClick))
	{
		if (event->getSender() == m_ok)
		{
			ui::CommandEvent cmdEvent(this, event->getItem(), ui::Command(DrOk));
			raiseEvent(EiClick, &cmdEvent);
		}
		else if (event->getSender() == m_cancel)
		{
			ui::CommandEvent cmdEvent(this, event->getItem(), ui::Command(DrCancel));
			raiseEvent(EiClick, &cmdEvent);
		}
		else if (event->getSender() == m_apply)
		{
			ui::CommandEvent cmdEvent(this, event->getItem(), ui::Command(DrApply));
			raiseEvent(EiClick, &cmdEvent);
		}
	}

	if (!event->consumed() && isModal())
	{
		if (event->getSender() == m_ok)
			endModal(DrOk);
		else if (event->getSender() == m_cancel)
			endModal(DrCancel);
	}
}

	}
}
