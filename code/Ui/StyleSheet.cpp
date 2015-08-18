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
	ss->setColor(L"traktor.ui.Widget", L"border-color", Color4ub(160, 160, 160));
	ss->setColor(L"traktor.ui.Dock", L"caption-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-focus", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Dock", L"caption-color-no-focus", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.Dock", L"splitter-color", Color4ub(204, 200, 219));
	ss->setColor(L"traktor.ui.Tab", L"tab-background-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-background-color-hover", Color4ub(28, 151, 234));
	ss->setColor(L"traktor.ui.Tab", L"tab-line-color", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-active", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.Tab", L"tab-color-inactive", Color4ub(30, 30, 30));
	ss->setColor(L"traktor.ui.custom.TreeView", L"background-color", Color4ub(246, 246, 246));
	ss->setColor(L"traktor.ui.custom.TreeView", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.custom.TreeView", L"item-background-color-selected", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.custom.GridView", L"background-color", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.custom.GridView", L"header-background-color", Color4ub(239, 239, 242));
	ss->setColor(L"traktor.ui.custom.GridView", L"item-color-selected", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.ui.custom.GridView", L"item-background-color-selected", Color4ub(51, 153, 255));
	ss->setColor(L"traktor.ui.custom.GridView", L"line-color", Color4ub(239, 239, 242));
	return ss;
}

Color4ub StyleSheet::getColor(const Widget* widget, const wchar_t* const element) const
{
	for (std::vector< Group >::const_reverse_iterator i = m_groups.rbegin(); i != m_groups.rend(); ++i)
	{
		if (i->element != element)
			continue;

		const TypeInfo* type = TypeInfo::find(i->type);
		if (!type)
			continue;

		if (is_type_of(*type, type_of(widget)))
			return i->color;
	}
	return Color4ub(255, 255, 255);
}

void StyleSheet::serialize(ISerializer& s)
{
	s >> MemberStlVector< Group, MemberComposite< Group > >(L"groups", m_groups);
}

void StyleSheet::setColor(const wchar_t* const type, const wchar_t* const element, const Color4ub& color)
{
	Group g;
	g.type = type;
	g.element = element;
	g.color = color;
	m_groups.push_back(g);
}

void StyleSheet::Group::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", type);
	s >> Member< std::wstring >(L"element", element);
	s >> Member< Color4ub >(L"color", color);
}

	}
}
