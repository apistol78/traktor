#ifndef traktor_ui_FormWx_H
#define traktor_ui_FormWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IForm.h"

namespace traktor
{
	namespace ui
	{

class FormWx : public WidgetWxImpl< IForm, wxFrame >
{
public:
	FormWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(ISystemBitmap* icon);

	virtual void maximize();

	virtual void minimize();

	virtual void restore();

	virtual bool isMaximized() const;

	virtual bool isMinimized() const;

	virtual void hideProgress();

	virtual void showProgress(int32_t current, int32_t total);

	virtual Rect getInnerRect() const;

	virtual void setOutline(const Point* p, int np);

	virtual void update(const Rect* rc, bool immediate);

private:
	bool m_unbufferedPaint;
	wxToolBar* m_toolBar;

	void onClose(wxCloseEvent& event);
};

	}
}

#endif	// traktor_ui_FormWx_H
