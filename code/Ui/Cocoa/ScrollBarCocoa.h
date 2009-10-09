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

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int range);

	virtual int getRange() const;

	virtual void setPage(int page);

	virtual int getPage() const;

	virtual void setPosition(int position);

	virtual int getPosition() const;
	
	// IWidget
	
	virtual Size getPreferedSize() const;
	
private:
	int m_range;
};

	}
}

#endif	// traktor_ui_ScrollBarCocoa_H
