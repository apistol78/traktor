/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

struct Group
{
	std::wstring type;
	std::wstring element;
	Color4ub color;

	void serialize(ISerializer& s)
	{
		s >> Member< std::wstring >(L"type", type);
		s >> Member< std::wstring >(L"element", element);
		s >> Member< Color4ub >(L"color", color);
	}
};

struct Value
{
	std::wstring name;
	std::wstring value;

	void serialize(ISerializer& s)
	{
		s >> Member< std::wstring >(L"name", name);
		s >> Member< std::wstring >(L"value", value);
	}
};

class MemberEntity : public MemberComplex
{
public:
	MemberEntity(const wchar_t* const name, StyleSheet::Entity& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< std::wstring >(L"typeName", m_ref.typeName);
		s >> MemberSmallMap< std::wstring, int32_t >(L"colors", m_ref.colors);
	}

private:
	StyleSheet::Entity& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.StyleSheet", 3, StyleSheet, ISerializable)

StyleSheet::Entity* StyleSheet::findEntity(const std::wstring& typeName)
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	return it != m_entities.end() ? &(*it) : nullptr;
}

StyleSheet::Entity* StyleSheet::addEntity(const std::wstring& typeName)
{
	auto& entity = m_entities.push_back();
	entity.typeName = typeName;
	return &entity;
}

int32_t StyleSheet::findColor(const Color4ub& color) const
{
	const auto it = std::find_if(m_palette.begin(), m_palette.end(), [&](const Color4ub& palette) {
		return palette == color;
	});
	return it != m_palette.end() ? (int32_t)std::distance(m_palette.begin(), it) : -1;
}

int32_t StyleSheet::addColor(const Color4ub& color)
{
	m_palette.push_back(color);
	return (int32_t)m_palette.size() - 1;
}

const Color4ub& StyleSheet::getColor(int32_t index) const
{
	return m_palette[index];
}

void StyleSheet::setColor(const std::wstring& typeName, const std::wstring_view& element, const Color4ub& color)
{
	int32_t index = findColor(color);
	if (index < 0)
		index = addColor(color);

	const auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	if (it != m_entities.end())
		it->colors[element] = index;
	else
	{
		auto& entity = m_entities.push_back();
		entity.typeName = typeName;
		entity.colors[element] = index;
	}
}

Color4ub StyleSheet::getColor(const std::wstring& typeName, const std::wstring_view& element) const
{
	const auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
		return entity.typeName == typeName;
	});
	if (it == m_entities.end())
		return Color4ub(255, 255, 255);

	const auto it2 = it->colors.find(element);
	if (it2 == it->colors.end())
		return Color4ub(255, 255, 255);

	return m_palette[it2->second];
}

Color4ub StyleSheet::getColor(const Object* widget, const std::wstring_view& element) const
{
	const TypeInfo* widgetType = widget ? &type_of(widget) : &type_of< Object >();
	while (widgetType != nullptr)
	{
		auto it = std::find_if(m_entities.begin(), m_entities.end(), [&](const Entity& entity) {
			return entity.typeName == widgetType->getName();
		});
		if (it != m_entities.end())
		{
			auto it2 = it->colors.find(element);
			if (it2 != it->colors.end())
				return m_palette[it2->second];
		}
		widgetType = widgetType->getSuper();
	}
	return Color4ub(255, 255, 255);
}

void StyleSheet::setValue(const std::wstring& name, const std::wstring_view& value)
{
	m_values[name] = value;
}

std::wstring StyleSheet::getValueRaw(const std::wstring_view& name) const
{
	const auto it = m_values.find(name);
	return it != m_values.end() ? it->second : L"";
}

std::wstring StyleSheet::getValue(const std::wstring_view& name) const
{
	auto it = m_values.find(name);
	if (it == m_values.end())
		return L"";

	std::wstring value = it->second;
	std::wstring tmp;

	size_t s = 0;
	for (;;)
	{
		s = value.find(L"$(", s);
		if (s == std::string::npos)
			break;

		size_t e = value.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		const std::wstring name = value.substr(s + 2, e - s - 2);
		if (!(tmp = getValueRaw(name)).empty())
			value = value.substr(0, s) + tmp + value.substr(e + 1);

		++s;
	}

	return value;
}

Ref< StyleSheet > StyleSheet::merge(const StyleSheet* right) const
{
	Ref< StyleSheet > ss = new StyleSheet();

	ss->m_entities = m_entities;
	for (const auto& entity : right->m_entities)
	{
		for (const auto& it : entity.colors)
		{
			const Color4ub& rclr = right->m_palette[it.second];
			ss->setColor(entity.typeName, it.first, rclr);
		}
	}

	ss->m_values = m_values;
	for (const auto& it : right->m_values)
		ss->setValue(it.first, it.second);

	return ss;
}

void StyleSheet::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< StyleSheet >() >= 3);
	s >> MemberAlignedVector< std::wstring >(L"include", m_include);
	s >> MemberAlignedVector< Entity, MemberEntity >(L"entities", m_entities);
	s >> MemberAlignedVector< Color4ub >(L"palette", m_palette);
	s >> MemberSmallMap< std::wstring, std::wstring >(L"values", m_values);
}

Ref< StyleSheet > StyleSheet::createDefault()
{
	Ref< StyleSheet > ss = new StyleSheet();

	ss->setColor(L"traktor.ui.Widget", L"color", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.Widget", L"color-disabled", Color4ub(60, 60, 60));
	ss->setColor(L"traktor.ui.Widget", L"background-color", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.Widget", L"background-color-disabled", Color4ub(230, 230, 235));
	ss->setColor(L"traktor.ui.Widget", L"border-color", Color4ub(160, 160, 160));

	ss->setColor(L"traktor.ui.ListBox", L"background-color", Color4ub(255, 255, 255));

	ss->setColor(L"traktor.ui.Edit", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Edit", L"background-color-disabled", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.Edit", L"border-color", Color4ub(180, 180, 180));
	ss->setColor(L"traktor.ui.Edit", L"border-color-disabled", Color4ub(180, 180, 180));

	ss->setColor(L"traktor.ui.Dock", L"caption-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-focus", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-no-focus", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.Dock", L"splitter-color", Color4ub(204, 200, 219));

	ss->setColor(L"traktor.ui.Tab", L"tab-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-background-color-hover", Color4ub(28, 151, 234));
	ss->setColor(L"traktor.ui.Tab", L"tab-line-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-active", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-inactive", Color4ub(30, 30, 30));

	ss->setColor(L"traktor.ui.BackgroundWorkerDialog", L"border-color", Color4ub(0, 122, 204));

	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-pushed", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-hover", Color4ub(254, 254, 254));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-toggled", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-seperator", Color4ub(160, 160, 160));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown-hover", Color4ub(254, 254, 254));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-dropdown-hover", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ToolBar", L"item-background-color-dropdown-button", Color4ub(243, 243, 246));
	ss->setColor(L"traktor.ui.ToolBar", L"item-color-dropdown-arrow", Color4ub(20, 20, 20));

	ss->setColor(L"traktor.ui.TreeView", L"background-color", Color4ub(246, 246, 246));
	ss->setColor(L"traktor.ui.TreeViewItem", L"color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.TreeViewItem", L"background-color-selected", Color4ub(51, 153, 255));

	ss->setColor(L"traktor.ui.GridView", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.GridView", L"header-background-color", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.GridView", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.GridView", L"item-background-color-selected", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.GridView", L"line-color", Color4ub(239, 239, 242));

	ss->setColor(L"traktor.ui.ScrollBar", L"background-color", Color4ub(219, 219, 222));
	ss->setColor(L"traktor.ui.ScrollBar", L"color-arrow", Color4ub(120, 120, 120));
	ss->setColor(L"traktor.ui.ScrollBar", L"color-slider", Color4ub(189, 189, 192));

	ss->setColor(L"traktor.ui.ListBox", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ListBox", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.ListBox", L"item-background-color-selected", Color4ub(51, 153, 255));

	ss->setColor(L"traktor.ui.LogList", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.LogList", L"background-color-info", Color4ub(255, 255, 255, 0));
	ss->setColor(L"traktor.ui.LogList", L"background-color-warning", Color4ub(255, 210, 87));
	ss->setColor(L"traktor.ui.LogList", L"background-color-error", Color4ub(255, 45, 45));
	ss->setColor(L"traktor.ui.LogList", L"color-info", Color4ub(0, 0, 0));
	ss->setColor(L"traktor.ui.LogList", L"color-warning", Color4ub(0, 0, 0));
	ss->setColor(L"traktor.ui.LogList", L"color-error", Color4ub(0, 0, 0));

	return ss;
}

	}
}
