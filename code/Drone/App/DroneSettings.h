/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drone_DroneSettings_H
#define traktor_drone_DroneSettings_H

#include <Core/Serialization/ISerializable.h>

namespace traktor
{
	namespace drone
	{

class DroneTool;

class DroneSettings : public ISerializable
{
	T_RTTI_CLASS

public:
	void addTool(DroneTool* tool);

	const RefArray< DroneTool >& getTools() const;

	virtual void serialize(ISerializer& s);

private:
	RefArray< DroneTool > m_tools;
};

	}
}

#endif	// traktor_drone_DroneSettings_H
