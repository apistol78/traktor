#ifndef traktor_ui_ToolFormX11_H
#define traktor_ui_ToolFormX11_H

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormX11 : public WidgetX11Impl< IToolForm >
{
public:
	ToolFormX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal() T_OVERRIDE T_FINAL;

	virtual void endModal(int result) T_OVERRIDE T_FINAL;

private:
	int32_t m_result;
	bool m_modal;
};

	}
}

#endif	// traktor_ui_ToolFormX11_H

