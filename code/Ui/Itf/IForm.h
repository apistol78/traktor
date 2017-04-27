/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IForm_H
#define traktor_ui_IForm_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;
class ISystemBitmap;

/*! \brief Form interface.
 * \ingroup UI
 */
class IForm : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;

	virtual void maximize() = 0;

	virtual void minimize() = 0;

	virtual void restore() = 0;

	virtual bool isMaximized() const = 0;

	virtual bool isMinimized() const = 0;

	virtual void hideProgress() = 0;

	virtual void showProgress(int32_t current, int32_t total) = 0;
};

	}
}

#endif	// traktor_ui_IForm_H
