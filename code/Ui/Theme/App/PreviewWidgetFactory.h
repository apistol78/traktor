#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace ui
	{

class StyleSheet;
class Widget;

class PreviewWidgetFactory : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Widget > create(Widget* parent, const StyleSheet* styleSheet, const std::wstring& typeName) const;
};

	}
}
