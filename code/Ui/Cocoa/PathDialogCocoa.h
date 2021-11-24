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

	virtual bool create(IWidget* parent, const std::wstring& title) override final;

	virtual void destroy() override final;

	virtual int showModal(Path& outPath) override final;

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

