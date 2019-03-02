#pragma once

#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class PsnGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);
};

	}
}

