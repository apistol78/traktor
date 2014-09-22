#include "Ui/Enums.h"
#include "Ui/Cocoa/PathDialogCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

PathDialogCocoa::PathDialogCocoa(EventSubject* owner)
:	m_owner(owner)
{
}

bool PathDialogCocoa::create(IWidget* parent, const std::wstring& title)
{
	m_openPanel = [NSOpenPanel openPanel];

	[m_openPanel setCanChooseFiles: NO];
	[m_openPanel setCanChooseDirectories: YES];

	return true;
}

void PathDialogCocoa::destroy()
{
	if (m_openPanel)
	{
		[m_openPanel orderOut: nil];
		m_openPanel = 0;
	}
}

int PathDialogCocoa::showModal(Path& outPath)
{
	[m_openPanel setAllowsMultipleSelection: NO];
	if ([m_openPanel runModal] == NSOKButton)
	{
		outPath = Path(fromNSString([m_openPanel filename]));
		return DrOk;
	}
	return DrCancel;
}

	}
}
