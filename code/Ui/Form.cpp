/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Form.h"
#include "Ui/MenuBar.h"
#include "Ui/Itf/IForm.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Form", Form, Container)

bool Form::create(const std::wstring& text, int width, int height, int style, Layout* layout, Widget* parent)
{
	IForm* form = Application::getInstance()->getWidgetFactory()->createForm(this);
	if (!form)
	{
		log::error << L"Failed to create native widget peer (Form)" << Endl;
		return false;
	}

	if (!form->create(parent ? parent->getIWidget() : 0, text, width, height, style))
	{
		form->destroy();
		return false;
	}

	m_widget = form;

	return Container::create(parent, style, layout);
}

void Form::setIcon(Bitmap* icon)
{
	T_ASSERT (m_widget);

	if (!icon || !icon->getSystemBitmap())
		return;

	static_cast< IForm* >(m_widget)->setIcon(icon->getSystemBitmap());
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

bool Form::acceptLayout() const
{
	return false;
}

	}
}
