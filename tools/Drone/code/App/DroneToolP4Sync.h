#ifndef traktor_drone_DroneToolP4Import_H
#define traktor_drone_DroneToolP4Import_H

#include <Core/Heap/Ref.h>
#include "App/DroneTool.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceClient;

class DroneToolP4Sync : public DroneTool
{
	T_RTTI_CLASS(DroneToolP4Sync)

public:
	DroneToolP4Sync();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_title;
	PerforceClientDesc m_clientDesc;
	bool m_verbose;
	Ref< PerforceClient > m_p4client;
};

	}
}

#endif	// traktor_drone_DroneToolP4Import_H
