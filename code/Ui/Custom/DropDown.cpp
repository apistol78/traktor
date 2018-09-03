/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Command.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/ToolForm.h"
#include "Ui/Custom/DropDown.h"
#include "Ui/Custom/ListBox/ListBox.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.DropDown", DropDown, Widget)

bool DropDown::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_buttonArrow = new Button();
	m_buttonArrow->create(this, L"...");
	m_buttonArrow->addEventHandler< ButtonClickEvent >(this, &DropDown::eventArrowClick);

	m_edit = new Edit();
	m_edit->create(this, L"", Edit::WsReadOnly);

	m_listForm = new ToolForm();
	m_listForm->create(this, L"", 0, 0, WsNone, new FloodLayout());
	m_listForm->setVisible(false);

	m_listBox = new ListBox();
	m_listBox->create(m_listForm);
	m_listBox->addEventHandler< MouseButtonDownEvent >(this, &DropDown::eventListButtonDown);

	addEventHandler< SizeEvent >(this, &DropDown::eventSize);
	return true;
}

int32_t DropDown::add(const std::wstring& item, Object* data)
{
	return m_listBox->add(item, data);
}

bool DropDown::remove(int32_t index)
{
	return m_listBox->remove(index);
}

void DropDown::removeAll()
{
	m_listBox->removeAll();
}

int32_t DropDown::count() const
{
	return m_listBox->count();
}

void DropDown::setItem(int32_t index, const std::wstring& item)
{
}

void DropDown::setData(int32_t index, Object* data)
{
}

std::wstring DropDown::getItem(int32_t index) const
{
	return L"";
}

Ref< Object > DropDown::getData(int32_t index) const
{
	return nullptr;
}

void DropDown::select(int32_t index)
{
}

bool DropDown::select(const std::wstring& item)
{
	return false;
}

int32_t DropDown::getSelected() const
{
	return -1;
}

std::wstring DropDown::getSelectedItem() const
{
	return std::wstring();
}

Ref< Object > DropDown::getSelectedData() const
{
	return nullptr;
}

Size DropDown::getPreferedSize() const
{
	int32_t h1 = m_edit->getPreferedSize().cy;
	int32_t h2 = m_buttonArrow->getPreferedSize().cy;
	return Size(dpi96(200), std::max(h1, h2));
}

void DropDown::eventArrowClick(ButtonClickEvent* event)
{
	m_listForm->show();
	m_listBox->setCapture();
}

void DropDown::eventListButtonDown(MouseButtonDownEvent* event)
{
	m_listBox->releaseCapture();
	m_listForm->hide();
}

void DropDown::eventSize(SizeEvent* event)
{
	const Size sz = event->getSize();
	const int32_t h = sz.cy;
	const int32_t lh = m_listBox->getItemHeight() * 8;

	m_edit->setRect(Rect(Point(0, 0), Size(sz.cx - h, h)));
	m_buttonArrow->setRect(Rect(Point(sz.cx - h, 0), Size(h, h)));
	m_listForm->setRect(Rect(Point(0, h), Size(sz.cx, lh)));
}

		}
	}
}
