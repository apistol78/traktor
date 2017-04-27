/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Form.h"
#include "Ui/MenuBar.h"
#include "Ui/Itf/IForm.h"
#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuBar", MenuBar, EventSubject)

MenuBar::MenuBar()
:	m_menuBar(0)
,	m_form(0)
{
}

MenuBar::~MenuBar()
{
	T_ASSERT_M (!m_menuBar, L"MenuBar not destroyed");
}

bool MenuBar::create(Form* form)
{
	T_ASSERT_M (!m_form, L"MenuBar already created");

	if (!form)
		return false;

	m_menuBar = Application::getInstance()->getWidgetFactory()->createMenuBar(this);
	if (!m_menuBar)
	{
		log::error << L"Failed to create native widget peer (MenuBar)" << Endl;
		return false;
	}

	if (!m_menuBar->create(static_cast< IForm* >(form->getIWidget())))
	{
		m_menuBar->destroy();
		m_menuBar = 0;
		return false;
	}

	m_form = form;
	m_form->update();

	return true;
}

void MenuBar::destroy()
{
	if (!m_menuBar)
		return;

	m_menuBar->destroy();
	m_menuBar = 0;

	m_form = 0;
}

void MenuBar::add(MenuItem* item)
{
	m_menuBar->add(item);
}

	}
}
