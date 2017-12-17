/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRefArray.h>
#include "Drone/App/DroneSettings.h"
#include "Drone/App/DroneTool.h"

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

void DroneSettings::serialize(ISerializer& s)
{
	s >> MemberRefArray< DroneTool >(L"tools", m_tools);
}

	}
}
