#ifndef traktor_drone_DroneToolP4Restore_H
#define traktor_drone_DroneToolP4Restore_H

#include <Core/Heap/Ref.h>
#include <Core/Io/Path.h>
#include "App/DroneTool.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceClient;

class DroneToolP4Restore : public DroneTool
{
	T_RTTI_CLASS(DroneToolP4Restore)

public:
	DroneToolP4Restore();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_title;
	PerforceClientDesc m_clientDesc;
	std::wstring m_backupPath;
	bool m_verbose;
	Ref< PerforceClient > m_p4client;
};

	}
}

#endif	// traktor_drone_DroneToolP4Restore_H
