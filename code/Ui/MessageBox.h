#ifndef traktor_ui_MessageBox_H
#define traktor_ui_MessageBox_H

#include "Core/Object.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class IMessageBox;

/*! \brief Message box.
 * \ingroup UI
 */
class T_DLLCLASS MessageBox : public EventSubject
{
	T_RTTI_CLASS;

public:
	MessageBox();

	virtual ~MessageBox();

	bool create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	void destroy();

	int showModal();

	static int show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	static int show(const std::wstring& message, const std::wstring& caption, int style);

private:
	IMessageBox* m_messageBox;
};

	}
}

#endif	// traktor_ui_MessageBox_H
