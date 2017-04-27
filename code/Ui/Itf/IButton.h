/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IButton_H
#define traktor_ui_IButton_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class Command;

/*! \brief Button interface.
 * \ingroup UI
 */
class IButton : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual void setState(bool state) = 0;

	virtual bool getState() const = 0;
};

	}
}

#endif	// traktor_ui_IButton_H
