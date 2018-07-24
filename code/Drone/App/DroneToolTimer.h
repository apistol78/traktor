/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneToolTimer_H
#define traktor_drone_DroneToolTimer_H

#include "Core/Timer/Timer.h"
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

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) T_OVERRIDE T_FINAL;

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_started;
	Timer m_timer;
	double m_elapsed;
};

	}
}

#endif	// traktor_drone_DroneToolTimer_H
