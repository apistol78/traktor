#ifndef traktor_drone_DroneTool_H
#define traktor_drone_DroneTool_H

#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

namespace traktor
{
	namespace ui
	{

class Widget;
class MenuItem;

	}

	namespace drone
	{

class DroneTool : public Serializable
{
	T_RTTI_CLASS(DroneTool)

public:
	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) = 0;

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem) = 0;
};

	}
}

#endif	// traktor_drone_DroneTool_H
