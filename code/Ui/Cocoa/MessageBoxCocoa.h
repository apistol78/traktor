#ifndef traktor_ui_MessageBoxCocoa_H
#define traktor_ui_MessageBoxCocoa_H

#include "Ui/Itf/IMessageBox.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MessageBoxCocoa : public IMessageBox
{
public:
	MessageBoxCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style);

	virtual void destroy();

	virtual int showModal();

private:
	std::wstring m_message;
	std::wstring m_caption;
};

	}
}

#endif	// traktor_ui_MessageBoxCocoa_H
