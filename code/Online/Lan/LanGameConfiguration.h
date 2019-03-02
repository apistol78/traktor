#pragma once

#include <list>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LanGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	LanGameConfiguration();

	virtual void serialize(ISerializer& s) override final;
};

	}
}

