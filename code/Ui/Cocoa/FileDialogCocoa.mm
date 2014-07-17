#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Ui/Enums.h"
#include "Ui/Cocoa/FileDialogCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

FileDialogCocoa::FileDialogCocoa(EventSubject* owner)
:	m_openPanel(0)
,	m_savePanel(0)
,	m_types(0)
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
		m_savePanel = [NSSavePanel savePanel];
		
		[m_savePanel setCanCreateDirectories: YES];
	}
	
	if (!filters.empty())
	{
		std::vector< std::wstring > fs;
		Split< std::wstring >::any(filters, L";", fs);
		
		if ((fs.size() & 1) != 0)
			return false;

		m_types = [[NSMutableArray alloc] init];
		
		for (uint32_t i = 0; i < uint32_t(fs.size()); i += 2)
		{
			std::wstring type = fs[i + 1];
			
			if (type == L"*.*")
				continue;
			
			if (startsWith< std::wstring >(type, L"*."))
				type = type.substr(2);
			
			[m_types addObject: makeNSString(type)];
		}
		
		if (![m_types count])
		{
			[m_types autorelease];
			m_types = 0;
		}
	}
	
	return true;
}

void FileDialogCocoa::destroy()
{
	if (m_savePanel)
	{
		[m_savePanel orderOut: nil];
		m_savePanel = 0;
	}

	if (m_openPanel)
	{
		[m_openPanel orderOut: nil];
		m_openPanel = 0;
	}
	
	if (m_types)
	{
		[m_types autorelease];
		m_types = 0;
	}
}

int FileDialogCocoa::showModal(Path& outPath)
{
	if (m_openPanel)
	{
		[m_openPanel setAllowsMultipleSelection: NO];
		if ([m_openPanel runModalForTypes: m_types] == NSOKButton)
		{
			NSArray* files = [m_openPanel filenames];
			if ([files count] > 0)
			{
				outPath = Path(fromNSString([files objectAtIndex: 0]));
				return DrOk;
			}
		}
	}
	else if (m_savePanel)
	{
		if ([m_savePanel runModal] == NSOKButton)
		{
			NSString* filename = [m_savePanel filename];
			if (filename)
			{
				outPath = Path(fromNSString(filename));
				return DrOk;
			}
		}
	}
	return DrCancel;
}

int FileDialogCocoa::showModal(std::vector< Path >& outPaths)
{
	if (m_openPanel)
	{
		[m_openPanel setAllowsMultipleSelection: YES];
		if ([m_openPanel runModalForTypes: m_types] == NSOKButton)
		{
			NSArray* files = [m_openPanel filenames];
			for (int i = 0; i < [files count]; ++i)
				outPaths.push_back(Path(fromNSString([files objectAtIndex: i])));
			return DrOk;
		}
	}
	else if (m_savePanel)
	{
		if ([m_savePanel runModal] == NSOKButton)
		{
			NSString* filename = [m_savePanel filename];
			if (filename)
			{
				outPaths.push_back(Path(fromNSString(filename)));
				return DrOk;
			}
		}
	}
	return DrCancel;
}
	
	}
}
