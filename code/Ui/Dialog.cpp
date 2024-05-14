/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Dialog.h"
#include "Ui/Form.h"
#include "Ui/IBitmap.h"
#include "Ui/Layout.h"
#include "Ui/Itf/IDialog.h"

namespace traktor::ui
{
	namespace
	{
		
template< typename WidgetType >
WidgetType* getAncestorOf(Widget* widget)
{
	if (!widget)
		return nullptr;

	if (is_a< WidgetType >(widget))
		return static_cast< WidgetType* >(widget);

	if (widget->getParent())
		return getAncestorOf< WidgetType >(widget->getParent());
	else
		return nullptr;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Dialog", Dialog, Container)

bool Dialog::create(Widget* parent, const std::wstring& text, Unit width, Unit height, uint32_t style, Layout* layout)
{
	IDialog* dialog = Application::getInstance()->getWidgetFactory()->createDialog(this);
	if (!dialog)
	{
		log::error << L"Failed to create native widget peer (Dialog)" << Endl;
		return false;
	}

	if (!dialog->create(parent ? parent->getIWidget() : nullptr, text, width.get(), height.get(), style))
	{
		dialog->destroy();
		return false;
	}

	// Ensure size are converted to display DPI.
	const int32_t w = dialog->dpi96(width.get());
	const int32_t h = dialog->dpi96(height.get());
	Rect rc = dialog->getRect();
	rc.setSize(ui::Size(w, h));
	dialog->setRect(rc);

	m_widget = dialog;

	if (!Container::create(parent, WsNone, layout))
		return false;

	// Get default icon from ancestor form.
	Form* ancestor = getAncestorOf< Form >(parent);
	if (ancestor)
		setIcon(ancestor->getIcon());

	addEventHandler< ChildEvent >(this, &Dialog::eventChild);
	return true;
}

void Dialog::setIcon(IBitmap* icon)
{
	T_ASSERT(m_widget);

	if (!icon || !icon->getSystemBitmap(this))
		return;

	static_cast< IDialog* >(m_widget)->setIcon(icon->getSystemBitmap(this));
}

DialogResult Dialog::showModal()
{
	T_ASSERT(m_widget);
	m_modal = true;
	update();
	return static_cast< IDialog* >(m_widget)->showModal();
}

void Dialog::endModal(DialogResult result)
{
	T_ASSERT(m_widget);
	T_ASSERT(m_modal);
	static_cast< IDialog* >(m_widget)->endModal(result);
	m_modal = false;
}

bool Dialog::isModal() const
{
	return m_modal;
}

bool Dialog::isEnable(bool includingParents) const
{
	T_ASSERT(m_widget);
	return m_widget->isEnable();
}

bool Dialog::acceptLayout() const
{
	return false;
}

void Dialog::eventChild(ChildEvent* event)
{
	if (event->getParent() != this || getLayout() == nullptr)
		return;

	const Size bounds = getInnerRect().getSize();
	Size result;
	if (!getLayout()->fit(this, bounds, result))
		return;

	static_cast< IDialog* >(m_widget)->setMinSize(result);
}

}
