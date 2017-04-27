/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IMessageBox_H
#define traktor_ui_IMessageBox_H

#include <string>
#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class IWidget;

/*! \brief MessageBox interface.
 * \ingroup UI
 */
class IMessageBox
{
public:
	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style) = 0;

	virtual void destroy() = 0;

	virtual int showModal() = 0;
};

	}
}

#endif	// traktor_ui_IMessageBox_H
