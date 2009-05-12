#ifndef traktor_drone_DroneSettings_H
#define traktor_drone_DroneSettings_H

#include <Core/Serialization/Serializable.h>

namespace traktor
{
	namespace drone
	{

class DroneTool;

class DroneSettings : public Serializable
{
	T_RTTI_CLASS(DroneSettings)

public:
	void addTool(DroneTool* tool);

	const RefArray< DroneTool >& getTools() const;

	virtual bool serialize(Serializer& s);

private:
	RefArray< DroneTool > m_tools;
};

	}
}

#endif	// traktor_drone_DroneSettings_H
