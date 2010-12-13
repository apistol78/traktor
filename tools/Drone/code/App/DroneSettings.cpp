#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRefArray.h>
#include "App/DroneSettings.h"
#include "App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drone.DroneSettings", 0, DroneSettings, ISerializable)

void DroneSettings::addTool(DroneTool* tool)
{
	m_tools.push_back(tool);
}

const RefArray< DroneTool >& DroneSettings::getTools() const
{
	return m_tools;
}

bool DroneSettings::serialize(ISerializer& s)
{
	return s >> MemberRefArray< DroneTool >(L"tools", m_tools);
}

	}
}
