#ifndef traktor_ui_ScrollBarWin32_H
#define traktor_ui_ScrollBarWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IScrollBar.h"

namespace traktor
{
	namespace ui
	{

class ScrollBarWin32 : public WidgetWin32Impl< IScrollBar >
{
public:
	ScrollBarWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int range);

	virtual int getRange() const;

	virtual void setPage(int page);

	virtual int getPage() const;

	virtual void setPosition(int position);

	virtual int getPosition() const;

	virtual Size getPreferedSize() const;

private:
	LRESULT eventScroll(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_ScrollBarWin32_H
