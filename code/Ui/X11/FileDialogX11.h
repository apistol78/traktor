#ifndef traktor_ui_FileDialogX11_H
#define traktor_ui_FileDialogX11_H

#include "Ui/Itf/IFileDialog.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class FileDialogX11 : public IFileDialog
{
public:
	FileDialogX11(EventSubject* owner);

	virtual ~FileDialogX11();

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal(Path& outPath) T_OVERRIDE T_FINAL;

	virtual int showModal(std::vector< Path >& outPaths) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
};

	}
}

#endif	// traktor_ui_FileDialogX11_H
