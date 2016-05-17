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

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal(Path& outPath) T_OVERRIDE T_FINAL;

	virtual int showModal(std::vector< Path >& outPaths) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
	NSOpenPanel* m_openPanel;
	NSSavePanel* m_savePanel;
	NSMutableArray* m_types;
};

	}
}

#endif	// traktor_ui_FileDialogCocoa_H
