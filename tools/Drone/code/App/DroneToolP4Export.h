#ifndef traktor_drone_DroneToolP4Export_H
#define traktor_drone_DroneToolP4Export_H

#include <Core/Ref.h>
#include <Core/Io/Path.h>
#include "App/DroneTool.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceClient;

class DroneToolP4Export : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolP4Export();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_title;
	PerforceClientDesc m_clientDesc;
	std::wstring m_exportPath;
	bool m_verbose;
	Ref< PerforceClient > m_p4client;
};

	}
}

#endif	// traktor_drone_DroneToolP4Export_H
