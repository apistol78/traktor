#pragma once

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

	virtual bool execute(ui::Widget* parent, const ui::MenuItem* menuItem) = 0;
};

	}
}

