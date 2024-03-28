/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IWidgetFactory.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Menu", Menu, Object)

void Menu::add(MenuItem* item)
{
	m_items.push_back(item);
}

void Menu::remove(MenuItem* item)
{
	m_items.remove(item);
}

void Menu::removeAll()
{
	m_items.clear();
}

const RefArray< MenuItem >& Menu::getItems() const
{
	return m_items;
}

Ref< Widget > Menu::show(Widget* parent, const Point& at) const
{
	if (!parent)
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0_ut, 0_ut, WsTop, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([=](MenuClickEvent* e) {
		MenuClickEvent clickEvent(form, e->getItem(), e->getCommand());
		form->raiseEvent(&clickEvent);
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit(Container::Both);

	// Place form at given position.
	const auto rc = form->getRect();
	Rect rcForm(
		parent->clientToScreen(at),
		rc.getSize()
	);

	// Ensure form is placed inside desktop.
	std::list< Rect > desktopRects;
	Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);
	auto it = std::find_if(desktopRects.begin(), desktopRects.end(), [&](const Rect& rc) {
		return rc.inside(rcForm.getTopLeft(), true);
	});
	if (it != desktopRects.end())
	{
		if (rcForm.left < it->left)
			rcForm = rcForm.offset(-(rcForm.left - it->left), 0);
		if (rcForm.right > it->right)
			rcForm = rcForm.offset(-(rcForm.right - it->right), 0);
		if (rcForm.top < it->top)
			rcForm = rcForm.offset(0, -(rcForm.top - it->top));
		if (rcForm.bottom > it->bottom)
			rcForm = rcForm.offset(0, -(rcForm.bottom - it->bottom));
	}

	// Show form.
	form->setRect(rcForm);
	form->show();

	return form;
}

const MenuItem* Menu::showModal(Widget* parent, const Point& at, int32_t width, int32_t maxItems) const
{
	const MenuItem* selectedItem = nullptr;

	if (!parent || m_items.empty())
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0_ut, 0_ut, WsNone, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form, maxItems))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
		selectedItem = e->getItem();
		form->endModal(DialogResult::Ok);
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit(Container::Both);

	// Place form at given position.
	auto rc = form->getRect();
	auto sz = rc.getSize();

	if (width >= 0)
		sz.cx = width;

	Rect rcForm(
		parent->clientToScreen(at),
		sz
	);

	// Ensure form is placed inside desktop.
	std::list< Rect > desktopRects;
	Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);
	auto it = std::find_if(desktopRects.begin(), desktopRects.end(), [&](const Rect& rc) {
		return rc.inside(rcForm.getTopLeft(), true);
	});
	if (it != desktopRects.end())
	{
		if (rcForm.left < it->left)
			rcForm = rcForm.offset(-(rcForm.left - it->left), 0);
		if (rcForm.right > it->right)
			rcForm = rcForm.offset(-(rcForm.right - it->right), 0);
		if (rcForm.top < it->top)
			rcForm = rcForm.offset(0, -(rcForm.top - it->top));
		if (rcForm.bottom > it->bottom)
			rcForm = rcForm.offset(0, -(rcForm.bottom - it->bottom));
	}
	form->setRect(rcForm);

	// Show form.
	form->showModal();

	safeDestroy(form);
	return selectedItem;
}

}
