#ifndef traktor_drone_DroneToolP4Import_H
#define traktor_drone_DroneToolP4Import_H

#include <Core/Ref.h>
#include "App/DroneTool.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceClient;

class DroneToolP4Sync : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolP4Sync();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_title;
	PerforceClientDesc m_clientDesc;
	bool m_verbose;
	Ref< PerforceClient > m_p4client;
};

	}
}

#endif	// traktor_drone_DroneToolP4Import_H
