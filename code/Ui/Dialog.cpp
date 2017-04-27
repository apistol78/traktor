/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Dialog.h"
#include "Ui/Layout.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Dialog", Dialog, Container)

Dialog::Dialog()
:	m_modal(false)
{
}

bool Dialog::create(Widget* parent, const std::wstring& text, int width, int height, int style, Layout* layout)
{
	IDialog* dialog = Application::getInstance()->getWidgetFactory()->createDialog(this);
	if (!dialog)
	{
		log::error << L"Failed to create native widget peer (Dialog)" << Endl;
		return false;
	}

	if (!dialog->create(parent ? parent->getIWidget() : 0, text, width, height, style))
	{
		dialog->destroy();
		return false;
	}
	
	m_widget = dialog;
	
	if (!Container::create(parent, WsNone, layout))
		return false;

	addEventHandler< ChildEvent >(this, &Dialog::eventChild);
	return true;
}

void Dialog::setIcon(drawing::Image* icon)
{
	T_ASSERT (m_widget);
	static_cast< IDialog* >(m_widget)->setIcon(icon);
}

int Dialog::showModal()
{
	T_ASSERT (m_widget);

	m_modal = true;

	update();
	return static_cast< IDialog* >(m_widget)->showModal();
}

void Dialog::endModal(int result)
{
	T_ASSERT (m_widget);
	T_ASSERT (m_modal);

	static_cast< IDialog* >(m_widget)->endModal(result);

	m_modal = false;
}

bool Dialog::isModal() const
{
	return m_modal;
}

bool Dialog::acceptLayout() const
{
	return false;
}

void Dialog::eventChild(ChildEvent* event)
{
	if (!getLayout())
		return;

	Size bounds = getInnerRect().getSize();
	Size result;
	if (!getLayout()->fit(this, bounds, result))
		return;

	static_cast< IDialog* >(m_widget)->setMinSize(result);
}

	}
}
