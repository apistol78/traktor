/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/ToolForm.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Menu", Menu, Object)

void Menu::add(MenuItem* item)
{
	m_items.push_back(item);
}

const RefArray< MenuItem >& Menu::getItems() const
{
	return m_items;
}

MenuItem* Menu::show(Widget* parent, const Point& at)
{
	Ref< MenuItem > selectedItem;

	if (!parent)
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0, 0, WsTop, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
		selectedItem = e->getItem();
		if (selectedItem != nullptr)
			form->endModal(DrOk);
		else
			form->endModal(DrCancel);
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit();

	// Place form at given position.
	auto rc = form->getRect();
	form->setRect(Rect(
		parent->clientToScreen(at),
		rc.getSize()
	));

	// Show form.
	form->show();

	// Modal until item selected.
	if (form->showModal() != DrOk)
		selectedItem = nullptr;

	return selectedItem;
}

	}
}
