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
#include "Ui/Form.h"
#include "Ui/Itf/IForm.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Form", Form, Container)

bool Form::create(const std::wstring& text, Unit width, Unit height, uint32_t style, Layout* layout, Widget* parent)
{
	IForm* form = Application::getInstance()->getWidgetFactory()->createForm(this);
	if (!form)
	{
		log::error << L"Failed to create native widget peer (Form)" << Endl;
		return false;
	}

	if (!form->create(parent ? parent->getIWidget() : nullptr, text, width.get(), height.get(), style))
	{
		form->destroy();
		return false;
	}

	// Ensure size are converted to display DPI.
	const int32_t w = form->dpi96(width.get());
	const int32_t h = form->dpi96(height.get());
	Rect rc = form->getRect();
	rc.setSize(ui::Size(w, h));
	form->setRect(rc);

	m_widget = form;
	m_icon = nullptr;
	return Container::create(parent, style, layout);
}

void Form::setIcon(IBitmap* icon)
{
	T_ASSERT(m_widget);

	if (!icon || !icon->getSystemBitmap(this))
		return;

	m_icon = icon;

	static_cast< IForm* >(m_widget)->setIcon(icon->getSystemBitmap(this));
}

IBitmap* Form::getIcon()
{
	return m_icon;
}

void Form::maximize()
{
	static_cast< IForm* >(m_widget)->maximize();
}

void Form::minimize()
{
	static_cast< IForm* >(m_widget)->minimize();
}

void Form::restore()
{
	static_cast< IForm* >(m_widget)->restore();
}

bool Form::isMaximized() const
{
	return static_cast< IForm* >(m_widget)->isMaximized();
}

bool Form::isMinimized() const
{
	return static_cast< IForm* >(m_widget)->isMinimized();
}

void Form::hideProgress()
{
	static_cast< IForm* >(m_widget)->hideProgress();
}

void Form::showProgress(int32_t current, int32_t total)
{
	static_cast< IForm* >(m_widget)->showProgress(current, total);
}

bool Form::isEnable(bool includingParents) const
{
	T_ASSERT(m_widget);
	return m_widget->isEnable();
}

bool Form::acceptLayout() const
{
	return false;
}

}
