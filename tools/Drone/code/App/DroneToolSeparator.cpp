#include <Ui/MenuItem.h>
#include "App/DroneToolSeparator.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.DroneToolSeparator", DroneToolSeparator, DroneTool)

void DroneToolSeparator::getMenuItems(RefArray< ui::MenuItem >& outItems)
{
	outItems.push_back(gc_new< ui::MenuItem >(L"-"));
}

bool DroneToolSeparator::execute(ui::Widget* parent, ui::MenuItem* menuItem)
{
	return false;
}

bool DroneToolSeparator::serialize(Serializer& s)
{
	return true;
}

	}
}
