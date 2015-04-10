#ifndef traktor_online_LanGameConfiguration_H
#define traktor_online_LanGameConfiguration_H

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

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_online_LanGameConfiguration_H
