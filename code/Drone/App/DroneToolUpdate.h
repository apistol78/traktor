/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneToolUpdate_H
#define traktor_drone_DroneToolUpdate_H

#include "Drone/App/DroneTool.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class BackgroundWorkerStatus;

		}
	}

	namespace drone
	{

class UpdateBundle;

class DroneToolUpdate : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolUpdate();

	virtual ~DroneToolUpdate();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) T_OVERRIDE T_FINAL;

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_url;
	std::wstring m_rootPath;

	void updateThread(ui::Widget* parent, UpdateBundle* bundle, ui::custom::BackgroundWorkerStatus* status);
};

	}
}

#endif	// traktor_drone_DroneToolUpdate_H
