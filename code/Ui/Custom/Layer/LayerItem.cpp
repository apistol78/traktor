/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Ui/Custom/Layer/LayerItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.LayerItem", LayerItem, Object)

LayerItem::LayerItem(const std::wstring& text, bool enable) :
	m_text(text),
	m_enable(enable),
	m_selected(false)
{
}

void LayerItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& LayerItem::getText() const
{
	return m_text;
}

void LayerItem::setEnable(bool enable)
{
	m_enable = enable;
	for (RefArray< LayerItem >::iterator i = m_childLayers.begin(); i != m_childLayers.end(); ++i)
		(*i)->setEnable(enable);
}

bool LayerItem::isEnabled() const
{
	return m_enable;
}

void LayerItem::setSelected(bool selected)
{
	m_selected = selected;
}

bool LayerItem::isSelected() const
{
	return m_selected;
}

Ref< LayerItem > LayerItem::getParentLayer()
{
	return m_parent;
}

void LayerItem::addChildLayer(LayerItem* childLayer)
{
	T_ASSERT_M (!childLayer->m_parent, L"Layer already has a parent");
	childLayer->m_parent = this;
	m_childLayers.push_back(childLayer);
}

void LayerItem::removeChildLayer(LayerItem* childLayer)
{
	T_ASSERT_M (childLayer->m_parent == this, L"Layer not a child of this layer");
	childLayer->m_parent = 0;

	RefArray< LayerItem >::iterator i = std::find(m_childLayers.begin(), m_childLayers.end(), childLayer);
	T_ASSERT_M (i != m_childLayers.end(), L"Child layer already removed");
	m_childLayers.erase(i);
}

RefArray< LayerItem >& LayerItem::getChildLayers()
{
	return m_childLayers;
}

		}
	}
}
