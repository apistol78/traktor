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

void StyleSheet::Group::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"type", type);
	s >> Member< std::wstring >(L"element", element);
	s >> Member< Color4ub >(L"color", color);
}

	}
}
