/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IMenuBar_H
#define traktor_ui_IMenuBar_H

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

class IForm;
class MenuItem;

/*! \brief MenuBar interface.
 * \ingroup UI
 */
class IMenuBar
{
public:
	virtual bool create(IForm* form) = 0;

	virtual void destroy() = 0;

	virtual void add(MenuItem* item) = 0;
};

	}
}

#endif	// traktor_ui_IMenuBar_H
