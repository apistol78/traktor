#pragma once

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

	virtual bool create(IWidget* parent, const std::wstring& title) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal(Path& outPath) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

