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
#include "Ui/Bitmap.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolForm", ToolForm, Container)

ToolForm::ToolForm()
:	m_modal(false)
{
}

bool ToolForm::create(Widget* parent, const std::wstring& text, int width, int height, int style, Layout* layout)
{
	IToolForm* toolForm = Application::getInstance()->getWidgetFactory()->createToolForm(this);
	if (!toolForm)
	{
		log::error << L"Failed to create native widget peer (ToolForm)" << Endl;
		return false;
	}

	Ref< Layout > refLayout = layout;

	if (!toolForm->create(parent ? parent->getIWidget() : 0, text, width, height, style))
	{
		toolForm->destroy();
		return false;
	}

	m_widget = toolForm;

	return Container::create(parent, style, refLayout);
}

void ToolForm::setIcon(IBitmap* icon)
{
	T_ASSERT(m_widget);

	if (!icon || !icon->getSystemBitmap())
		return;

	m_icon = icon;

	static_cast< IToolForm* >(m_widget)->setIcon(icon->getSystemBitmap());
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

bool ToolForm::acceptLayout() const
{
	return false;
}

	}
}
