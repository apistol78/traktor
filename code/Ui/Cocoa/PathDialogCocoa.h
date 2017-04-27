/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool create(IWidget* parent, const std::wstring& title) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal(Path& outPath) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

#endif	// traktor_ui_PathDialogCocoa_H
