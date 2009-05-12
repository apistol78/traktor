#ifndef traktor_drone_DroneToolP4Backup_H
#define traktor_drone_DroneToolP4Backup_H

#include <Core/Heap/Ref.h>
#include <Core/Io/Path.h>
#include "App/DroneTool.h"
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

class PerforceClient;

class DroneToolP4Backup : public DroneTool
{
	T_RTTI_CLASS(DroneToolP4Backup)

public:
	DroneToolP4Backup();

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

#endif	// traktor_drone_DroneToolP4Backup_H
