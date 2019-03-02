#pragma once

#include "Drone/App/DroneTool.h"

namespace traktor
{
	namespace ui
	{

class BackgroundWorkerStatus;

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

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems) override final;

	virtual bool execute(ui::Widget* parent, const ui::MenuItem* menuItem) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_url;
	std::wstring m_rootPath;

	void updateThread(ui::Widget* parent, UpdateBundle* bundle, ui::BackgroundWorkerStatus* status);
};

	}
}

