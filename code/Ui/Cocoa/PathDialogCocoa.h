#ifndef traktor_ui_PathDialogCocoa_H
#define traktor_ui_PathDialogCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IPathDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class PathDialogCocoa : public IPathDialog
{
public:
	PathDialogCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& title);

	virtual void destroy();

	virtual int showModal(Path& outPath);

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

#endif	// traktor_ui_PathDialogCocoa_H
