#pragma once

#include "CustomTool.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace sb
	{

class Solution;

class ImportMsvcProject : public CustomTool
{
	T_RTTI_CLASS;

public:
	bool execute(ui::Widget* parent, Solution* solution, const std::wstring& solutionFileName);
};

	}
}

