#include <Ui/MenuItem.h>
#include "App/DroneToolSeparator.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneToolSeparator", 0, DroneToolSeparator, DroneTool)

void DroneToolSeparator::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	outItems.push_back(new ui::MenuItem(L"-"));
}

bool DroneToolSeparator::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	return false;
}

void DroneToolSeparator::serialize(ISerializer& s)
{
}

	}
}
