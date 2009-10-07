#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/MessageBoxCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{
	
MessageBoxCocoa::MessageBoxCocoa(EventSubject* owner)
{
}

bool MessageBoxCocoa::create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	m_message = message;
	m_caption = caption;
	return true;
}

void MessageBoxCocoa::destroy()
{
}

int MessageBoxCocoa::showModal()
{
	int result = NSRunAlertPanel(
		makeNSString(m_caption),
		makeNSString(m_message),
		@"OK", @"Cancel", NULL
	);

	if (result == NSAlertDefaultReturn)
		return DrOk;
	if (result == NSAlertAlternateReturn)
		return DrCancel;

	return DrCancel;
}
	
	}
}
