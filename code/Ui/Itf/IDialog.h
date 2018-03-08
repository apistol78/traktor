/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IDialog_H
#define traktor_ui_IDialog_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;

/*! \brief Dialog interface.
 * \ingroup UI
 */
class IDialog : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) = 0;

	virtual void setIcon(ISystemBitmap* icon) = 0;
	
	virtual int showModal() = 0;

	virtual void endModal(int result) = 0;

	virtual void setMinSize(const Size& minSize) = 0;
};

	}
}

#endif	// traktor_ui_IDialog_H
