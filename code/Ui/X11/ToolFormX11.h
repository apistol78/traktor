#pragma once

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormX11 : public WidgetX11Impl< IToolForm >
{
public:
	explicit ToolFormX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void destroy() override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual DialogResult showModal() override final;

	virtual void endModal(DialogResult result) override final;

private:
	DialogResult m_result;
	bool m_modal;
};

	}
}
