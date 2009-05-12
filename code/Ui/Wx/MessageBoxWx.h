#ifndef traktor_ui_MessageBoxWx_H
#define traktor_ui_MessageBoxWx_H

#include <wx/wx.h>
#include "Ui/Itf/IMessageBox.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MessageBoxWx : public IMessageBox
{
public:
	MessageBoxWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style);

	virtual void destroy();

	virtual int showModal();

private:
	wxMessageDialog* m_messageBox;
};

	}
}

#endif	// traktor_ui_MessageBoxWx_H
