/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Button.h"
#include "Ui/Edit.h"
#include "Ui/Command.h"
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

	m_edit = new Edit();
	m_edit->create(this, L"", Edit::WsReadOnly);

	m_listBox = new ListBox();
	m_listBox->create(this);

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

void DropDown::eventSize(SizeEvent* event)
{
	Size sz = event->getSize();
	m_edit->setRect(Rect(Point(0, 0), Size(sz.cx - 16, 16)));
	m_buttonArrow->setRect(Rect(Point(sz.cx - 16, 0), Size(16, 16)));
	m_listBox->setRect(Rect(Point(0, 16), Size(sz.cx, sz.cy - 16)));
}

		}
	}
}
