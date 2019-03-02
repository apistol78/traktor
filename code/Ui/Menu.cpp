#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MenuShell.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IWidgetFactory.h"

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

Ref< Widget > Menu::show(Widget* parent, const Point& at) const
{
	if (!parent)
		return nullptr;

	Ref< ToolForm > form = new ToolForm();
	if (!form->create(parent, L"", 0, 0, WsTop, new FloodLayout()))
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
	form->fit();

	// Place form at given position.
	auto rc = form->getRect();
	Rect rcForm(
		parent->clientToScreen(at),
		rc.getSize()
	);

	// Ensure form is placed inside desktop.
	std::list< Rect > desktopRects;
	Application::getInstance()->getWidgetFactory()->getDesktopRects(desktopRects);
	for (auto r : desktopRects)
	{
		if (rcForm.left < r.left)
			rcForm = rcForm.offset(-(rcForm.left - r.left), 0);
		if (rcForm.right > r.right)
			rcForm = rcForm.offset(-(rcForm.right - r.right), 0);
		if (rcForm.top < r.top)
			rcForm = rcForm.offset(0, -(rcForm.top - r.top));
		if (rcForm.bottom > r.bottom)
			rcForm = rcForm.offset(0, -(rcForm.bottom - r.bottom));
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
	if (!form->create(parent, L"", 0, 0, WsTop, new FloodLayout()))
		return nullptr;

	Ref< MenuShell > shell = new MenuShell();
	if (!shell->create(form, maxItems))
		return nullptr;

	shell->addEventHandler< MenuClickEvent >([&](MenuClickEvent* e) {
		selectedItem = e->getItem();
		form->endModal(DrOk);
	});

	for (auto item : m_items)
		shell->add(item);

	// Resize form to shell size.
	form->fit();

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
	for (auto r : desktopRects)
	{
		if (rcForm.left < r.left)
			rcForm = rcForm.offset(-(rcForm.left - r.left), 0);
		if (rcForm.right > r.right)
			rcForm = rcForm.offset(-(rcForm.right - r.right), 0);
		if (rcForm.top < r.top)
			rcForm = rcForm.offset(0, -(rcForm.top - r.top));
		if (rcForm.bottom > r.bottom)
			rcForm = rcForm.offset(0, -(rcForm.bottom - r.bottom));
	}

	form->setRect(rcForm);

	// Show form.
	form->showModal();

	safeDestroy(form);
	return selectedItem;
}

	}
}
