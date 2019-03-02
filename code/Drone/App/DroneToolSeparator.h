#pragma once

#include "Drone/App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

class DroneToolSeparator : public DroneTool
{
	T_RTTI_CLASS

public:
	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) override final;

	virtual bool execute(ui::Widget* parent, const ui::MenuItem* menuItem) override final;

	virtual void serialize(ISerializer& s) override final;
};

	}
}

