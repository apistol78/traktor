#ifndef traktor_ui_DialogX11_H
#define traktor_ui_DialogX11_H

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogX11 : public WidgetX11Impl< IDialog >
{
public:
	DialogX11(EventSubject* owner, Display* display, int32_t screen);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon) T_OVERRIDE T_FINAL;

	virtual int showModal() T_OVERRIDE T_FINAL;

	virtual void endModal(int result) T_OVERRIDE T_FINAL;

	virtual void setMinSize(const Size& minSize) T_OVERRIDE T_FINAL;

private:
	int32_t m_result;
	bool m_modal;
};

	}
}

#endif	// traktor_ui_DialogX11_H
