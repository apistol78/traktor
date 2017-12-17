/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneToolTimer_H
#define traktor_drone_DroneToolTimer_H

#include <Core/Timer/Timer.h>
#include "Drone/App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

class DroneToolTimer : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolTimer();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual void serialize(ISerializer& s);

private:
	bool m_started;
	Timer m_timer;
	double m_elapsed;
};

	}
}

#endif	// traktor_drone_DroneToolTimer_H
