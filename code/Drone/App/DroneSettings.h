#pragma once

#include "Core/Serialization/ISerializable.h"

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

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< DroneTool > m_tools;
};

	}
}

