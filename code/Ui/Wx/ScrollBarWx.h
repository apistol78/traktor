/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ScrollBarWx_H
#define traktor_ui_ScrollBarWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IScrollBar.h"

namespace traktor
{
	namespace ui
	{

class ScrollBarWx : public WidgetWxImpl< IScrollBar, wxScrollBar >
{
public:
	ScrollBarWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int range);

	virtual int getRange() const;

	virtual void setPage(int page);

	virtual int getPage() const;

	virtual void setPosition(int position);

	virtual int getPosition() const;

private:
	void onScroll(wxScrollEvent& event);
};

	}
}

#endif	// traktor_ui_ScrollBarWx_H
