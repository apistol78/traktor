#ifndef traktor_ui_FileDialogCocoa_H
#define traktor_ui_FileDialogCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/IFileDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class FileDialogCocoa : public IFileDialog
{
public:
	FileDialogCocoa(EventSubject* owner);

	virtual ~FileDialogCocoa();

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save);

	virtual void destroy();

	virtual int showModal(Path& outPath);

	virtual int showModal(std::vector< Path >& outPaths);

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
};

	}
}

#endif	// traktor_ui_FileDialogCocoa_H
