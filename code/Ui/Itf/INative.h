/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_INative_H
#define traktor_ui_INative_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Native interface.
 * \ingroup UI
 */
class INative : public IWidget
{
public:
	virtual bool create(void* nativeHandle) = 0;
};

	}
}

#endif	// traktor_ui_INative_H
