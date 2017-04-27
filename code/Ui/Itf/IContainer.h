/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IContainer_H
#define traktor_ui_IContainer_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Container interface.
 * \ingroup UI
 */
class IContainer : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;
};

	}
}

#endif	// traktor_ui_IContainer_H
