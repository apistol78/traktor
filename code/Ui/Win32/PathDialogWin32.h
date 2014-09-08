#ifndef traktor_ui_PathDialogWin32_H
#define traktor_ui_PathDialogWin32_H

#include "Core/Misc/TString.h"
#include "Ui/Itf/IPathDialog.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief
 * \ingroup UIW32
 */
class PathDialogWin32 : public IPathDialog
{
public:
	PathDialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& title);

	virtual void destroy();

	virtual int showModal(Path& outPath);

private:
	EventSubject* m_owner;
	Window m_hWnd;

	BROWSEINFO m_bi;
	TCHAR m_title[64];
};

	}
}

#endif	// traktor_ui_PathDialogWin32_H
