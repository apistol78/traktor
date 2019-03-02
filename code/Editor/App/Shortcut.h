#pragma once

#include <string>
#include <utility>
#include "Ui/Enums.h"

namespace traktor
{
	namespace editor
	{

std::pair< int, ui::VirtualKey > parseShortcut(const std::wstring& keyDesc);

std::wstring describeShortcut(const std::pair< int, ui::VirtualKey >& shortcut);

	}
}

