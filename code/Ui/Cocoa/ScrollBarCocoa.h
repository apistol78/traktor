#ifndef traktor_ui_ScrollBarCocoa_H
#define traktor_ui_ScrollBarCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IScrollBar.h"

namespace traktor
{
	namespace ui
	{

class ScrollBarCocoa : public WidgetCocoaImpl< IScrollBar, NSScroller >
{
public:
	ScrollBarCocoa(EventSubject* owner);
	
	// IScrollBar

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;

	virtual void setRange(int range) T_OVERRIDE T_FINAL;

	virtual int getRange() const T_OVERRIDE T_FINAL;

	virtual void setPage(int page) T_OVERRIDE T_FINAL;

	virtual int getPage() const T_OVERRIDE T_FINAL;

	virtual void setPosition(int position) T_OVERRIDE T_FINAL;

	virtual int getPosition() const T_OVERRIDE T_FINAL;
	
	// IWidget
	
	virtual Size getPreferedSize() const T_OVERRIDE T_FINAL;
	
private:
	bool m_vertical;
	int m_range;
	int m_page;
	
	void callbackAction(void* controldId);
};

	}
}

#endif	// traktor_ui_ScrollBarCocoa_H
