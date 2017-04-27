/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/ListViewItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListViewItem", ListViewItem, Object)

void ListViewItem::setText(int columnIndex, const std::wstring& text)
{
	if (columnIndex >= int(m_items.size()))
		m_items.resize(columnIndex + 1);

	m_items[columnIndex].text = text;
}

std::wstring ListViewItem::getText(int columnIndex) const
{
	if (columnIndex < 0 || columnIndex >= int(m_items.size()))
		return std::wstring();

	return m_items[columnIndex].text;
}

void ListViewItem::setImage(int columnIndex, int imageIndex)
{
	if (columnIndex >= int(m_items.size()))
		m_items.resize(columnIndex + 1);

	m_items[columnIndex].image = imageIndex;
}

int ListViewItem::getImage(int columnIndex) const
{
	if (columnIndex < 0 || columnIndex >= int(m_items.size()))
		return -1;

	return m_items[columnIndex].image;
}

int ListViewItem::getColumnCount() const
{
	return int(m_items.size());
}

	}
}
