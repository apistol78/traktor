/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IScrollBar_H
#define traktor_ui_IScrollBar_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief ScrollBar interface.
 * \ingroup UI
 */
class IScrollBar : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual void setRange(int range) = 0;

	virtual int getRange() const = 0;

	virtual void setPage(int page) = 0;

	virtual int getPage() const = 0;

	virtual void setPosition(int position) = 0;

	virtual int getPosition() const = 0;
};

	}
}

#endif	// traktor_ui_IScrollBar_H
