/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolForm.h"

#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Itf/IToolForm.h"
#include "Ui/StyleSheet.h"

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

	m_widget = toolForm;
	if (!Container::create(parent, style, refLayout))
		return false;

	addEventHandler< PaintEvent >(this, &ToolForm::eventPaint);
	return true;
}

void ToolForm::setIcon(IBitmap* icon)
{
	T_ASSERT(m_widget);

	if (!icon || !icon->getSystemBitmap(this))
		return;

	m_icon = icon;

	static_cast< IToolForm* >(m_widget)->setIcon(icon->getSystemBitmap(this));
}

IBitmap* ToolForm::getIcon() const
{
	return m_icon;
}

void ToolForm::setLayerImage(IBitmap* layerImage)
{
	T_ASSERT(m_widget);

	if (layerImage && layerImage->getSystemBitmap(this))
		static_cast< IToolForm* >(m_widget)->setLayerImage(layerImage->getSystemBitmap(this));
	else
		static_cast< IToolForm* >(m_widget)->setLayerImage(nullptr);

	m_layerImage = layerImage;
}

IBitmap* ToolForm::getLayerImage() const
{
	return m_layerImage;
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

void ToolForm::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(event->getUpdateRect());
}

}
