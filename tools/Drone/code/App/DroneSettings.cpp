#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/MemberRef.h>
#include "App/DroneSettings.h"
#include "App/DroneTool.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.DroneSettings", DroneSettings, Serializable)

void DroneSettings::addTool(DroneTool* tool)
{
	m_tools.push_back(tool);
}

const RefArray< DroneTool >& DroneSettings::getTools() const
{
	return m_tools;
}

bool DroneSettings::serialize(Serializer& s)
{
	return s >> MemberRefArray< DroneTool >(L"tools", m_tools);
}

	}
}
