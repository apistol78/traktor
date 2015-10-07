#ifndef traktor_editor_Shortcut_H
#define traktor_editor_Shortcut_H

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

#endif	// traktor_editor_Shortcut_H
