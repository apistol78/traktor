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

DropDown::DropDown()
{
}

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
	m_listBox->setItem(index, item);
}

void DropDown::setData(int32_t index, Object* data)
{
	m_listBox->setData(index, data);
}

std::wstring DropDown::getItem(int32_t index) const
{
	return m_listBox->getItem(index);
}

Ref< Object > DropDown::getData(int32_t index) const
{
	return m_listBox->getData(index);
}

void DropDown::select(int32_t index)
{
	m_listBox->select(index);
	m_edit->setText(m_listBox->getSelectedItem());
}

bool DropDown::select(const std::wstring& item)
{
	for (int32_t i = 0; i < m_listBox->count(); ++i)
	{
		if (m_listBox->getItem(i) == item)
		{
			select(i);
			return true;
		}
	}
	select(-1);
	return false;
}

int32_t DropDown::getSelected() const
{
	return m_listBox->getSelected();
}

std::wstring DropDown::getSelectedItem() const
{
	return m_listBox->getSelectedItem();
}

Ref< Object > DropDown::getSelectedData() const
{
	return m_listBox->getSelectedData();
}

Size DropDown::getPreferedSize() const
{
	int32_t h1 = m_edit->getPreferedSize().cy;
	int32_t h2 = m_buttonArrow->getPreferedSize().cy;
	return Size(dpi96(200), std::max(h1, h2));
}

void DropDown::eventArrowClick(ButtonClickEvent* event)
{
	const Size sz = getRect().getSize();
	const int32_t h = sz.cy;
	const int32_t lh = m_listBox->getItemHeight() * 8;

	m_listForm->setRect(Rect(clientToScreen(Point(0, h)), Size(sz.cx, lh)));
	m_listForm->show();

	m_listBox->setCapture();
}

void DropDown::eventListButtonDown(MouseButtonDownEvent* event)
{
	m_listBox->releaseCapture();
	m_listForm->hide();
	m_edit->setText(m_listBox->getSelectedItem());
}

void DropDown::eventSize(SizeEvent* event)
{
	const Size sz = event->getSize();
	const int32_t h = sz.cy;

	m_edit->setRect(Rect(Point(0, 0), Size(sz.cx - h, h)));
	m_buttonArrow->setRect(Rect(Point(sz.cx - h, 0), Size(h, h)));
}

		}
	}
}
