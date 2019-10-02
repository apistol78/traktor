#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.StyleSheet", 0, StyleSheet, ISerializable)

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
	ss->setColor(L"traktor.ui.TreeView", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.TreeView", L"item-background-color-selected", Color4ub(51, 153, 255));

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

Color4ub StyleSheet::getColor(const Object* widget, const wchar_t* const element) const
{
	for (auto it = m_groups.rbegin(); it != m_groups.rend(); ++it)
	{
		if (it->element != element)
			continue;

		const TypeInfo* type = TypeInfo::find(it->type.c_str());
		if (!type)
			continue;

		if (is_type_of(*type, type_of(widget)))
			return it->color;
	}
	return Color4ub(255, 255, 255);
}

std::wstring StyleSheet::getValue(const wchar_t* const name) const
{
	for (const auto& value : m_values)
	{
		if (value.name == name)
			return value.value;
	}
	return L"";
}

Ref< StyleSheet > StyleSheet::merge(const StyleSheet* right) const
{
	Ref< StyleSheet > ss = new StyleSheet();

	ss->m_groups = m_groups;
	for (auto group : right->m_groups)
		ss->setColor(group.type.c_str(), group.element.c_str(), group.color);

	ss->m_values = m_values;
	for (auto value : right->m_values)
		ss->setValue(value.name.c_str(), value.value.c_str());

	return ss;
}

void StyleSheet::serialize(ISerializer& s)
{
	s >> MemberStlVector< Group, MemberComposite< Group > >(L"groups", m_groups);
	s >> MemberStlVector< Value, MemberComposite< Value > >(L"values", m_values);
}

void StyleSheet::setColor(const wchar_t* const type, const wchar_t* const element, const Color4ub& color)
{
	for (auto& group : m_groups)
	{
		if (group.type == type && group.element == element)
		{
			group.color = color;
			return;
		}
	}

	// If we reach here then no such group.
	Group g;
	g.type = type;
	g.element = element;
	g.color = color;
	m_groups.push_back(g);
}

void StyleSheet::setValue(const wchar_t* const name, const wchar_t* const value)
{
	for (auto& v : m_values)
	{
		if (v.name == name)
		{
			v.value = value;
			return;
		}
	}

	// If we reach here then no such value.
	Value v;
	v.name = name;
	v.value = value;
	m_values.push_back(v);
}

void StyleSheet::Group::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", type);
	s >> Member< std::wstring >(L"element", element);
	s >> Member< Color4ub >(L"color", color);
}

void StyleSheet::Value::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"value", value);
}

	}
}
