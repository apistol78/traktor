#include "Ui/Cocoa/FileDialogCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

FileDialogCocoa::FileDialogCocoa(EventSubject* owner)
:	m_openPanel(0)
{
}

FileDialogCocoa::~FileDialogCocoa()
{
}

bool FileDialogCocoa::create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	if (!save)
	{
		m_openPanel = [NSOpenPanel openPanel];
	
		[m_openPanel setCanChooseFiles: YES];
		[m_openPanel setCanChooseDirectories: NO];
	}
	else
	{
		// @fixme
	}
	return true;
}

void FileDialogCocoa::destroy()
{
	if (m_openPanel)
	{
		[m_openPanel orderOut: nil];
		m_openPanel = 0;
	}
}

int FileDialogCocoa::showModal(Path& outPath)
{
	[m_openPanel setAllowsMultipleSelection: NO];
	if ([m_openPanel runModalForTypes: nil] == NSOKButton)
	{
		NSArray* files = [m_openPanel filenames];
		if ([files count] > 0)
		{
			outPath = Path(fromNSString([files objectAtIndex: 0]));
			return DrOk;
		}
		else
			return DrCancel;
	}
	else
		return DrCancel;
}

int FileDialogCocoa::showModal(std::vector< Path >& outPaths)
{
	[m_openPanel setAllowsMultipleSelection: YES];
	if ([m_openPanel runModalForTypes: nil] == NSOKButton)
	{
		NSArray* files = [m_openPanel filenames];
		for (int i = 0; i < [files count]; ++i)
			outPaths.push_back(Path(fromNSString([files objectAtIndex: i])));
		return DrOk;
	}
	else
		return DrCancel;
}
	
	}
}
