#ifndef traktor_ui_FormX11_H
#define traktor_ui_FormX11_H

#include "Ui/Itf/IForm.h"
#include "Ui/X11/WidgetX11Impl.h"

namespace traktor
{
	namespace ui
	{

class FormX11 : public WidgetX11Impl< IForm >
{
public:
	FormX11(EventSubject* owner, Display* display, int32_t screen);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon) T_OVERRIDE T_FINAL;

	virtual void maximize() T_OVERRIDE T_FINAL;

	virtual void minimize() T_OVERRIDE T_FINAL;

	virtual void restore() T_OVERRIDE T_FINAL;

	virtual bool isMaximized() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual void hideProgress() T_OVERRIDE T_FINAL;

	virtual void showProgress(int32_t current, int32_t total) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_FormX11_H

