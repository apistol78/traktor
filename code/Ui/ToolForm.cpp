/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolForm", ToolForm, Container)

bool ToolForm::create(Widget* parent, const std::wstring& text, Unit width, Unit height, uint32_t style, Layout* layout)
{
	IToolForm* toolForm = Application::getInstance()->getWidgetFactory()->createToolForm(this);
	if (!toolForm)
	{
		log::error << L"Failed to create native widget peer (ToolForm)" << Endl;
		return false;
	}

	Ref< Layout > refLayout = layout;

	if (!toolForm->create(parent ? parent->getIWidget() : 0, text, width.get(), height.get(), style))
	{
		toolForm->destroy();
		return false;
	}

	//// Ensure size are converted to display DPI.
	//const int32_t w = toolForm->dpi96(width.get());
	//const int32_t h = toolForm->dpi96(height.get());
	//Rect rc = toolForm->getRect();
	//rc.setSize(ui::Size(w, h));
	//toolForm->setRect(rc);

	m_widget = toolForm;
	return Container::create(parent, style, refLayout);
}

void ToolForm::setIcon(IBitmap* icon)
{
	T_ASSERT(m_widget);

	if (!icon || !icon->getSystemBitmap(this))
		return;

	m_icon = icon;

	static_cast< IToolForm* >(m_widget)->setIcon(icon->getSystemBitmap(this));
}

IBitmap* ToolForm::getIcon()
{
	return m_icon;
}

DialogResult ToolForm::showModal()
{
	T_ASSERT(m_widget);
	m_modal = true;
	update();
	return static_cast< IToolForm* >(m_widget)->showModal();
}

void ToolForm::endModal(DialogResult result)
{
	T_ASSERT(m_widget);
	T_ASSERT(m_modal);
	static_cast< IToolForm* >(m_widget)->endModal(result);
	m_modal = false;
}

bool ToolForm::isModal() const
{
	return m_modal;
}

bool ToolForm::isEnable(bool includingParents) const
{
	T_ASSERT(m_widget);
	return m_widget->isEnable();
}

bool ToolForm::acceptLayout() const
{
	return false;
}

}
