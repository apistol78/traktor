/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ISlider_H
#define traktor_ui_ISlider_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Slider interface.
 * \ingroup UI
 */
class ISlider : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual void setRange(int minValue, int maxValue) = 0;

	virtual void setValue(int value) = 0;

	virtual int getValue() const = 0;
};

	}
}

#endif	// traktor_ui_ISlider_H
