/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneTool_H
#define traktor_drone_DroneTool_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class MenuItem;

	}

	namespace drone
	{

class DroneTool : public ISerializable
{
	T_RTTI_CLASS

public:
	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) = 0;

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem) = 0;
};

	}
}

#endif	// traktor_drone_DroneTool_H
