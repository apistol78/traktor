#ifndef traktor_ui_DialogX11_H
#define traktor_ui_DialogX11_H

#include "Ui/Itf/IDialog.h"
#include "Ui/X11/WidgetX11Impl.h"

namespace traktor
{
	namespace ui
	{

class DialogX11 : public WidgetX11Impl< IDialog >
{
public:
	DialogX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon) T_OVERRIDE T_FINAL;

	virtual int showModal() T_OVERRIDE T_FINAL;

	virtual void endModal(int result) T_OVERRIDE T_FINAL;

	virtual void setMinSize(const Size& minSize) T_OVERRIDE T_FINAL;

private:
	Atom m_atomWmDeleteWindow;
	int32_t m_result;
	bool m_modal;
};

	}
}

#endif	// traktor_ui_DialogX11_H
